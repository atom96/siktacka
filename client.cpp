#include <chrono>
#include <poll.h>
#include <iostream>
#include "client.h"

static const size_t serverIn = 0;
static const size_t serverOut = 1;
static const size_t guiIn = 2;
static const size_t guiOut = 3;
static const size_t pollTableSize = 4;
static const unsigned int maxTimeout = 20;

void Client::preparePollTable(pollfd pollTable[]) {
    pollTable[serverIn].fd = serverConnection.getSocket();
    pollTable[serverIn].events = POLLIN;
    pollTable[serverIn].revents = 0;

    pollTable[serverOut].fd = serverConnection.getSocket();
    pollTable[serverOut].events = POLLOUT;
    pollTable[serverOut].revents = 0;

    pollTable[guiIn].fd = guiConnection.getSocket();
    pollTable[guiIn].events = POLLIN;
    pollTable[guiIn].revents = 0;

    pollTable[guiOut].fd = guiConnection.getSocket();
    pollTable[guiOut].events = POLLOUT;
    pollTable[guiOut].revents = 0;
}

bool Client::shouldISentMessageToServer(milliseconds lastTimeISentMessageToServer) {
    auto diff = currentTime() - lastTimeISentMessageToServer;
    return diff.count() > maxTimeout;
}

void Client::sendMessageToServerIfNeededAndPossible(
        milliseconds& lastTimeISentMessageToServer,
        pollfd pollTable[],
        ClientState& clientState
) {

    if (shouldISentMessageToServer(lastTimeISentMessageToServer) &&
        (pollTable[serverOut].revents & POLLOUT)) {
        static char messageBuffer[1000];

        char turnDirection = getDirection(
                clientState.isLeftKeyPressed,
                clientState.isRightKeyPressed,
                clientState.wasRightKeyPressedLast
        );

        MessageToServer message(
                clientState.sessionID,
                turnDirection,
                clientState.nextExpectedEventNumber,
                std::shared_ptr<std::string>(name));
        size_t size = message.toBytes(messageBuffer);

        try {
            serverConnection.sendMessage(messageBuffer, size);
            std::cout << "UMIERAM!!!!";
            exit(ERROR);
        }
        catch (SendException& e) {
            std::cerr << std::string(e.what()) << std::endl;
            exit(ERROR);
        }

        lastTimeISentMessageToServer = currentTime();
    }
}

void Client::sendMessageToGUI(ClientState& clientState) {
    while (clientState.messegesToSend.size() > 0) {
        std::string& eventText = clientState.messegesToSend.front();

        try {
            guiConnection.sendMessage(eventText.c_str(), eventText.size());
            clientState.messegesToSend.pop();
        }
        catch (SendException& e) {
            std::cerr << e.what() << std::endl;
            exit(ERROR);
        }
    }
}

void Client::getMessageFromGUI(ClientState& clientState) {
    static const int maxMessageSize = 15;
    char message[maxMessageSize + 5]; // just to be safe
    try {
        size_t size = guiConnection.getMessage(message, maxMessageSize);
        MessageFromGUI messageFromGUI(message, size);
        switch (messageFromGUI.getStatus()) {
            case LEFT_KEY_DOWN:
                clientState.isLeftKeyPressed = true;
                clientState.wasRightKeyPressedLast = false;
                break;
            case LEFT_KEY_UP:
                clientState.isLeftKeyPressed = false;
                break;
            case RIGHT_KEY_DOWN:
                clientState.isRightKeyPressed = true;
                clientState.wasRightKeyPressedLast = true;
                break;
            case RIGHT_KEY_UP:
                clientState.isRightKeyPressed = false;
                break;
            default:
                std::cerr
                        << "Somehow function returned another enum. Message ignored"
                        << std::endl;
        }
    }
    catch (ReceiveException& e) {
        std::cerr
                << "Problem with connection " + std::string(e.what())
                << std::endl;
        exit(ERROR);
    }
    catch (std::exception& e) {
        std::cerr
                << "Message will be ignored " + std::string(e.what())
                << std::endl;
    }
}

void Client::acceptMessageFromServer(ClientState& clientState) {
    static const size_t maxLength = 513;

    char messageBuffer[maxLength + 5]; //just to be safe
    size_t size = 0;

    try {
        size = serverConnection.getMessage(messageBuffer, maxLength);

        if (size > 512) {
            std::cerr << "Too long message" << std::endl;
            return;
        }

        MessageFromServer message(messageBuffer, size);
        message.prepareEvents(clientState);
    }
    catch (ReceiveException& e) {
        std::cerr << std::string(e.what()) << std::endl;
        exit(ERROR);
    }
    catch (InvalidPartnerException& e) {
        std::cout << std::string(e.what()) << std::endl;
        return;
    }
    catch (ConstructorException& e) {
        std::cerr
                << "Constructor exception has happened " + std::string(e.what())
                << std::endl;
    }
}

char Client::getDirection(
        bool isLeftKeyPressed, bool isRightKeyPressed, bool wasRightPressedLast
) {
    char turnDirection;
    if (isRightKeyPressed && isLeftKeyPressed) {
        if (wasRightPressedLast) {
            turnDirection = 1;
        }
        else {
            turnDirection = -1;
        }
    }
    else if (isRightKeyPressed) {
        turnDirection = 1;
    }
    else if (isLeftKeyPressed) {
        turnDirection = -1;
    }
    else {
        turnDirection = 0;
    }

    return turnDirection;
}

void Client::startCommunication() {
    pollfd pollTable[pollTableSize];
    unsigned int timeout = maxTimeout;
    ClientState myState;
    milliseconds lastTimeISentMessageToServer = milliseconds(0);
    while (true) {
        preparePollTable(pollTable);

        int ret = poll(pollTable, pollTableSize, timeout);
        if (ret < 0) {
            std::cerr << "Poll error" << std::endl;
            exit(ERROR);
        }

        sendMessageToServerIfNeededAndPossible(
                lastTimeISentMessageToServer, pollTable, myState
        );

        if (pollTable[serverIn].revents & POLLIN) {
            acceptMessageFromServer(myState);
        }

        sendMessageToServerIfNeededAndPossible(
                lastTimeISentMessageToServer, pollTable, myState
        );
        if (myState.messegesToSend.size() > 0 &&
            pollTable[guiOut].revents & POLLOUT) {
            sendMessageToGUI(myState);
        }

        sendMessageToServerIfNeededAndPossible(
                lastTimeISentMessageToServer, pollTable, myState
        );

        if (pollTable[guiIn].revents & POLLIN) {
            getMessageFromGUI(myState);
        }

        sendMessageToServerIfNeededAndPossible(
                lastTimeISentMessageToServer, pollTable, myState
        );

        auto diff = currentTime() - lastTimeISentMessageToServer;

        long diff2 = maxTimeout - diff.count();

        timeout = static_cast<unsigned int>(std::max(diff2, 0l));
    }
}
