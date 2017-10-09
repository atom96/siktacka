#include <cstring>
#include <iostream>
#include "messeges.h"
#include <zlib.h>
#include "helper.h"

uint32_t calculateControlSum(char* bufferToCalculate, size_t bufferSize) {
    return (uint32_t) crc32(0, (Bytef*) bufferToCalculate, bufferSize);
}

bool checkControlSum(
        const char* bufferToCheck, size_t bufferSize, uint32_t controlSum
) {
    return controlSum ==
           (uint32_t) crc32(0, (Bytef*) bufferToCheck, bufferSize);
}

/*===========================================================================*
 *                        Class: MessageToServer                             *
 *===========================================================================*/

MessageToServer::MessageToServer(
        uint64_t currentSession,
        char direction,
        uint32_t nexExpectedEventNumber,
        std::shared_ptr<std::string> name
)
        :
        sessionID(currentSession),
        turnDirection(direction),
        nextExpectedEvent(nexExpectedEventNumber),
        playerName(name) {
}


void MessageToServer::parseSessionID(
        const char*& encodedMessage, size_t& messageLength
) {

    if (messageLength < sizeof sessionID) {
        throw MessageSizeException("Message too short to contain session ID");
    }
    sessionID = be64toh(*(uint64_t*) encodedMessage);
    encodedMessage += sizeof sessionID;
    messageLength -= sizeof sessionID;
}

void MessageToServer::parseTurnDirection(
        const char*& encodedMessage, size_t& messageLength
) {
    if (messageLength < sizeof turnDirection) {
        throw MessageSizeException(
                "Message too short to contain turn direction"
        );
    }

    turnDirection = *encodedMessage;
    encodedMessage += sizeof turnDirection;
    messageLength -= sizeof turnDirection;

    if (turnDirection != 0 && turnDirection != 1 && turnDirection != -1) {
        throw InvalidArgumentException("Read invalid turn direction");
    }
}

void MessageToServer::parseNextExpectedEvent(
        const char*& encodedMessage, size_t& messageLength
) {

    if (messageLength < sizeof nextExpectedEvent) {
        throw MessageSizeException(
                "Message too short to contain next expected event"
        );
    }

    nextExpectedEvent = be32toh(*(uint32_t*) encodedMessage);
    encodedMessage += sizeof nextExpectedEvent;
    messageLength -= sizeof nextExpectedEvent;
}

void MessageToServer::parsePlayerName(
        const char*& encodedMessage, size_t& messageLength
) {
    if (messageLength <= 0) {
        throw MessageSizeException("Message too short to contain player name");
    }

    if (encodedMessage[messageLength - 1] != '\0') {
        ((char*) encodedMessage)[messageLength] = '\0';
    }

    playerName = std::shared_ptr<std::string>(new std::string(encodedMessage));

    if (!isValidPlayerName(*playerName)) {
        throw InvalidArgumentException("Invalid player name");
    }
}


MessageToServer::MessageToServer(
        const char* encodedMessage, size_t messageLength
) {

    if (encodedMessage == nullptr) {
        throw NullPointerException("Encoded message cannot be NULL");
    }

    parseSessionID(encodedMessage, messageLength);
    parseTurnDirection(encodedMessage, messageLength);
    parseNextExpectedEvent(encodedMessage, messageLength);
    parsePlayerName(encodedMessage, messageLength);
}

void MessageToServer::encodeSessionID(
        char*& bufferForMessage, uint32_t& totalSize
) {
    uint64_t encodedID = htobe64(sessionID);
    memcpy((void*) bufferForMessage, (void*) &encodedID, sizeof encodedID);
    bufferForMessage += sizeof encodedID;
    totalSize += sizeof encodedID;

}

void MessageToServer::encodeTurnDirection(
        char*& bufferForMessage, uint32_t& totalSize
) {
    *bufferForMessage = turnDirection;
    bufferForMessage += sizeof turnDirection;
    totalSize += sizeof turnDirection;

}

void MessageToServer::encodeNextExpectedEvent(
        char*& bufferForMessage, uint32_t& totalSize
) {
    uint32_t encodedEvent = htobe32((uint32_t) nextExpectedEvent);
    memcpy((void*) bufferForMessage, (void*) &encodedEvent, sizeof encodedEvent
    );
    bufferForMessage += sizeof encodedEvent;
    totalSize += sizeof encodedEvent;

}

void MessageToServer::encodePlayerName(
        char*& bufferForMessage, uint32_t& totalSize
) {
    memcpy((void*) bufferForMessage,
           (void*) playerName->c_str(),
           playerName->length() + 1
    );
    totalSize += playerName->length();//+ 1;
}

uint32_t MessageToServer::toBytes(char* bufferForMessage) {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Cannot write to NULL buffer");
    }
    uint32_t totalSize = 0;

    encodeSessionID(bufferForMessage, totalSize);
    encodeTurnDirection(bufferForMessage, totalSize);
    encodeNextExpectedEvent(bufferForMessage, totalSize);
    encodePlayerName(bufferForMessage, totalSize);

    return totalSize;
}

bool MessageToServer::operator==(const MessageToServer& m) const {
    return sessionID == m.sessionID &&
           turnDirection == m.turnDirection &&
           nextExpectedEvent == m.nextExpectedEvent &&
           playerName == m.playerName;
}

std::string MessageToServer::toString() {
    return "Session ID: " +
           numberToString(sessionID) +
           "\n" +
           "Turn direcion: " +
           numberToString((int) turnDirection) +
           "\n" +
           "Next expected event: " +
           numberToString(nextExpectedEvent) +
           "\nName: " +
           *playerName;
}

uint64_t MessageToServer::getSessionID() {
    return sessionID;
}

char MessageToServer::getDirection() {
    return turnDirection;
}

std::shared_ptr<std::string>& MessageToServer::getPlayerName() {
    return playerName;
}

uint32_t MessageToServer::getExpectedEventNumber() {
    return nextExpectedEvent;
}

/*===========================================================================*
 *                        Class: MessageFromServer                           *
 *===========================================================================*/

MessageFromServer::MessageFromServer(
        uint32_t newID, std::queue<EventPair>&& newEvents
) : gameID(newID), events(std::move(newEvents)) {
}

void MessageFromServer::parseGameID(
        const char*& encodedMessage, size_t& messageLength
) {
    if (messageLength < sizeof gameID) {
        throw InvalidArgumentException("Message too short");
    }
    gameID = be32toh(*(uint32_t*) encodedMessage);
    encodedMessage += sizeof gameID;
    messageLength -= sizeof gameID;
}

uint32_t MessageFromServer::parseDataLength(
        const char*& encodedMessage, size_t& messageLength
) {
    uint32_t dataLength;

    if (messageLength < sizeof dataLength) {
        throw MessageSizeException(
                "Message is too short : " + numberToString(messageLength));
    }
    dataLength = be32toh(*(uint32_t*) encodedMessage);
    encodedMessage += sizeof dataLength;
    messageLength -= sizeof dataLength;
    return dataLength;
}

uint32_t MessageFromServer::parseControlSum(
        const char*& encodedMessage, size_t& messageLength, uint32_t dataLength
) {
    uint32_t controlSum;

    if (messageLength < dataLength + sizeof controlSum) {
        throw MessageSizeException(
                "Message is too short : " +
                numberToString(messageLength) +
                " expected: " +
                numberToString(
                        dataLength + sizeof controlSum
                ));
    }

    controlSum = be32toh(*(uint32_t*) (encodedMessage + dataLength));

    return controlSum;
}

void assertControlSumCorrectness(
        const char* messageStart, size_t structLength, uint32_t sumToCheck
) {
    if (!checkControlSum(messageStart, structLength, sumToCheck)) {
        throw ControlSumException(
                "Control sum is not correct. Got: " +
                numberToString(sumToCheck) +
                "Expected: " +
                numberToString(sumToCheck));
    }
}


uint32_t MessageFromServer::parseEventNumber(
        const char*& encodedMessage, size_t& messageLength
) {
    uint32_t eventNumber = be32toh(*(uint32_t*) encodedMessage);
    encodedMessage += sizeof eventNumber;
    messageLength -= sizeof eventNumber;
    return eventNumber;
}

char MessageFromServer::parseEventType(
        const char*& encodedMessage, size_t& messageLength
) {
    char eventType = *encodedMessage;
    encodedMessage += sizeof eventType;
    messageLength -= sizeof eventType;

    return eventType;
}

void MessageFromServer::parseEventStructure(
        const char*& encodedMessage, size_t& messageLength
) {


    const char* messageStart = encodedMessage;
    std::shared_ptr<AbstractEvent> event;
    uint32_t eventNumber;
    char eventType;
    uint32_t dataLength;
    uint32_t controlSum;
    dataLength = parseDataLength(encodedMessage, messageLength);

    controlSum = parseControlSum(encodedMessage, messageLength, dataLength);

    assertControlSumCorrectness(
            messageStart, sizeof dataLength + dataLength, controlSum
    );

    eventNumber = parseEventNumber(encodedMessage, messageLength);

    eventType = parseEventType(encodedMessage, messageLength);

    dataLength -= sizeof eventType + sizeof eventNumber;

    switch (eventType) {
        case newGameID :
            event = std::shared_ptr<AbstractEvent>(
                    new NewGameEvent(encodedMessage, dataLength));
            break;
        case pixelID:
            event = std::shared_ptr<AbstractEvent>(
                    new PixelEvent(encodedMessage, dataLength));
            break;
        case playerEliminatedID:
            event = std::shared_ptr<AbstractEvent>(
                    new PlayerEliminatedEvent(encodedMessage, dataLength));
            break;
        case gameOverID:
            event = std::shared_ptr<AbstractEvent>(
                    new GameOverEvent(encodedMessage, dataLength));
            break;
        default:
            encodedMessage += dataLength;
            messageLength -= dataLength;
            encodedMessage += sizeof controlSum;
            messageLength -= sizeof controlSum;
            throw EventTypeException("Invalid event type");
    }

    encodedMessage += dataLength;
    messageLength -= dataLength;
    encodedMessage += sizeof controlSum;
    messageLength -= sizeof controlSum;
    events.push(std::make_pair(std::move(event), eventNumber));
}


MessageFromServer::MessageFromServer(
        const char* encodedMessage, size_t messageLength
) {
    if (encodedMessage == nullptr) {
        throw NullPointerException("Buffer cannot be NULL");
    }

    parseGameID(encodedMessage, messageLength);

    while (messageLength > 0) {
        try {
            parseEventStructure(encodedMessage, messageLength);

        }
        catch (ControlSumException& e) {
            std::cerr
                    << "Control sum is not valid, stop parsing events " +
                       std::string(e.what())
                    << std::endl;
            return;
        }
        catch (EventTypeException& e) {
            std::cerr
                    << "Control sum valid but argument type is invalid " +
                       std::string(e.what())
                    << std::endl;
            continue;
        }
        catch (InvalidArgumentException& e) {
            std::cerr
                    <<
                    "Correct control sum and event type but wrong data. Client will die right now " +
                    std::string(e.what())
                    << std::endl;
            exit(ERROR);
        }
        catch (MessageSizeException& e) {
            throw ConstructorException(
                    "Invalid message size. Previous message: " +
                    std::string(e.what()));
        }
        catch (std::exception& e) {
            throw ConstructorException(
                    "Unexpected exception. Client will ignore this message " +
                    std::string(e.what()));
        }
    }

    if (messageLength != 0) {
        throw UnexpectedSituationException("Message size is negative");
    }
}

void MessageFromServer::encodeGameID(
        char*& bufferForMessage, uint32_t& totalSize
) {
    uint32_t encodedID = htobe32(gameID);
    memcpy((void*) bufferForMessage, (void*) &encodedID, sizeof encodedID);
    bufferForMessage += sizeof encodedID;
    totalSize += sizeof encodedID;
}


void MessageFromServer::encodeControlSum(
        char*& bufferForMessage, uint32_t& eventSize, char* bufferStart
) {
    uint32_t controlSum = calculateControlSum(bufferStart, eventSize);

    uint32_t encodedSum = htobe32(controlSum);

    memcpy((void*) bufferForMessage, (void*) &encodedSum, sizeof encodedSum);
    bufferForMessage += sizeof encodedSum;

    eventSize += sizeof controlSum;
}

void MessageFromServer::encodeDataSize(
        char*& bufferStart, uint32_t eventFieldsLength
) {
    uint32_t encodedLength = htobe32(eventFieldsLength);
    memcpy((void*) bufferStart, (void*) &encodedLength, sizeof encodedLength);
}

void MessageFromServer::encodeEventNumber(
        char*& bufferForMessage,
        uint32_t& eventFieldsLenght,
        uint32_t eventNumber
) {
    uint32_t encodedEventNumber = htobe32(eventNumber);
    memcpy((void*) bufferForMessage,
           (void*) &encodedEventNumber,
           sizeof encodedEventNumber
    );
    bufferForMessage += sizeof encodedEventNumber;
    eventFieldsLenght += sizeof encodedEventNumber;
}

uint32_t MessageFromServer::encodeEventStruct(
        char*& bufferForMessage, EventPair& eventPair
) {
    char* bufferStart = bufferForMessage;
    uint32_t eventSize = 0;
    uint32_t eventFieldsLength = 0;
    std::shared_ptr<AbstractEvent>& event = eventPair.first;
    uint32_t eventNumber = eventPair.second;

    bufferForMessage += sizeof eventFieldsLength;
    eventSize += sizeof eventFieldsLength;

    encodeEventNumber(bufferForMessage, eventFieldsLength, eventNumber);
    uint32_t dataSize = event->toBytes(bufferForMessage);

    eventFieldsLength = dataSize + sizeof eventFieldsLength;
    eventSize += eventFieldsLength;
    bufferForMessage += dataSize;

    encodeDataSize(bufferStart, eventFieldsLength);
    encodeControlSum(bufferForMessage, eventSize, bufferStart);

    return eventSize;
}


uint32_t MessageFromServer::toBytes(char* bufferForMessage) {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Cannot write to NULL buffer");
    }

    uint32_t totalSize = 0;

    encodeGameID(bufferForMessage, totalSize);

    //std::cout << totalSize <<std::endl;
    while (events.size() > 0) {
        EventPair& eventPair = events.front();

        uint32_t eventSize = encodeEventStruct(bufferForMessage, eventPair);
        if (eventSize + totalSize < maxMessageSize) {
            totalSize += eventSize;
            events.pop();
        }
        else {
            break;
        }
    }

    return totalSize;
}

std::queue<EventPair>&& MessageFromServer::giveEvents() {
    return std::move(events);
}

size_t MessageFromServer::getEventsNumber() {
    return events.size();
}


void MessageFromServer::prepareEvents(ClientState& clientState) {
    if (clientState.gameID != gameID &&
        clientState.nextExpectedEventNumber != 0) {
        std::cerr << "Wrong game ID, ignoring message" << std::endl;
        return;
    }

    clientState.expectedGameID = gameID;

    while (!events.empty()) {
        EventPair& eventPair = events.front();
        std::shared_ptr<AbstractEvent>& event = eventPair.first;
        uint32_t eventNumber = eventPair.second;

        event->changeClientState(clientState, eventNumber);

        events.pop();
    }
}

/*===========================================================================*
 *                        Class: MessageFromGUI                              *
 *===========================================================================*/

MessageFromGUI::MessageFromGUI(char* encodedMessage, size_t messageSize) {
    if (encodedMessage[messageSize - 1] != '\n') {
        throw InvalidArgumentException(
                "Expected end of line at the end of message. Got:" +
                std::string(message));
    }

    encodedMessage[messageSize - 1] = '\0';

    message = std::string(encodedMessage);
    if (message != "LEFT_KEY_DOWN" &&
        message != "LEFT_KEY_UP" &&
        message != "RIGHT_KEY_DOWN" &&
        message != "RIGHT_KEY_UP") {
        throw InvalidArgumentException(
                "Message is not right. Got:" +
                message +
                numberToString((unsigned int) message[messageSize - 1]));
    }
}

KeyStatus MessageFromGUI::getStatus() {
    if (message == "LEFT_KEY_DOWN") {
        return LEFT_KEY_DOWN;
    }
    if (message == "LEFT_KEY_UP") {
        return LEFT_KEY_UP;
    }
    if (message == "RIGHT_KEY_DOWN") {
        return RIGHT_KEY_DOWN;
    }
    if (message == "RIGHT_KEY_UP") {
        return RIGHT_KEY_UP;
    }

    throw UnexpectedSituationException(
            "Somehow message has inappropriate content"
    );
}