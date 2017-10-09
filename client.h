#ifndef SIK_DUZE_CLIENT_H
#define SIK_DUZE_CLIENT_H

#include <poll.h>
#include <chrono>
#include <set>
#include "tcpmessenger.h"
#include "udpmessenger.h"
#include "messeges.h"

// Class which represents game client. Is responsible for dealing with communication
// both with server and GUI.

class Client {
private:
    TCPMessenger guiConnection;
    UDPMessenger serverConnection;
    std::shared_ptr<std::string> name;

    void preparePollTable(pollfd pollTable[]);

    bool shouldISentMessageToServer(milliseconds lastTimeISentMessageToServer);

    void sendMessageToServerIfNeededAndPossible(
            milliseconds& lastTimeISentMessageToServer,
            pollfd pollTable[],
            ClientState& clientState
    );

    void getMessageFromGUI(ClientState& clientState);

    void acceptMessageFromServer(ClientState& clientState);

    char getDirection(
            bool isLeftKeyPressed,
            bool isRightKeyPressed,
            bool wasRightPressedLast
    );

    void sendMessageToGUI(ClientState& clientState);

public:
    // Main constructor. We require all of the fields to be specified.
    Client(
            TCPMessenger&& tcpPartner,
            UDPMessenger udpPartner,
            std::shared_ptr<std::string> playerName
    ) : guiConnection(
            std::move(tcpPartner)),
        serverConnection(udpPartner),
        name(playerName) {
    }

    //executes main client loop
    void startCommunication();
};

#endif //SIK_DUZE_CLIENT_H
