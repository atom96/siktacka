//
// Created by arek on 02.06.17.
//

#ifndef SIK_DUZE_MESSEGES_H
#define SIK_DUZE_MESSEGES_H

#include <memory>
#include <queue>
#include "events.h"
#include "server_state.h"

static const char newGameID = 0;
static const char pixelID = 1;
static const char playerEliminatedID = 2;
static const char gameOverID = 3;

/*===========================================================================*
 *                        Class: MessageToServer                             *
 *===========================================================================*/
// represents message sent from client to server
class MessageToServer {
private:
    uint64_t sessionID;
    char turnDirection;
    uint32_t nextExpectedEvent;
    std::shared_ptr<std::string> playerName;

    void parseSessionID(const char*& encodedMessage, size_t& messageLength);

    void parseTurnDirection(const char*& encodedMessage, size_t& messageLength);

    void parseNextExpectedEvent(
            const char*& encodedMessage, size_t& messageLength
    );

    void parsePlayerName(const char*& encodedMessage, size_t& messageLength);

    void encodeSessionID(char*& bufferForMessage, uint32_t& totalSize);

    void encodeTurnDirection(char*& bufferForMessage, uint32_t& totalSize);

    void encodeNextExpectedEvent(char*& bufferForMessage, uint32_t& totalSize);

    void encodePlayerName(char*& bufferForMessage, uint32_t& totalSize);

public:
    //construct itself from buffer
    MessageToServer(const char* encodedMessage, size_t messageLength);

    //standard constructor
    MessageToServer(
            uint64_t currentSession,
            char direction,
            uint32_t nexExpectedEventNumber,
            std::shared_ptr<std::string> name
    );

    //encode itself to bytes
    uint32_t toBytes(char* bufferForMessage);

    bool operator==(const MessageToServer& m) const;

    std::string toString();

    uint64_t getSessionID();

    char getDirection();

    std::shared_ptr<std::string>& getPlayerName();

    uint32_t getExpectedEventNumber();
};


/*===========================================================================*
 *                        Class: MessageFromServer                           *
 *===========================================================================*/

using EventPair = std::pair<std::shared_ptr<AbstractEvent>, uint32_t>;

//represents message from server to client
class MessageFromServer {
private:
    uint32_t gameID;
    std::queue<EventPair> events;
    static const int maxMessageSize = 512;

    void parseEventStructure(
            const char*& encodedMessage, size_t& messageLength
    );

    uint32_t parseDataLength(
            const char*& encodedMessage, size_t& messageLength
    );

    uint32_t parseEventNumber(
            const char*& encodedMessage, size_t& messageLength
    );

    char parseEventType(const char*& encodedMessage, size_t& messageLength);

    uint32_t parseControlSum(
            const char*& encodedMessage,
            size_t& messageLength,
            uint32_t dataLength
    );

    void parseGameID(const char*& encodedMessage, size_t& messageLength);

    void encodeGameID(char*& bufferForMessage, uint32_t& totalSize);

    uint32_t encodeEventStruct(char*& bufferForMessage, EventPair& eventPair);

    void encodeControlSum(
            char*& bufferForMessage, uint32_t& eventSize, char* bufferStart
    );

    void encodeDataSize(char*& bufferStart, uint32_t eventFieldsLength);

    void encodeEventNumber(
            char*& bufferForMessage,
            uint32_t& eventFieldsLenght,
            uint32_t eventNumber
    );

public:
    //standard constructor
    MessageFromServer(uint32_t newID, std::queue<EventPair>&& newEvent);

    //create itself from bytes
    MessageFromServer(const char* encodedMessage, size_t messageLength);

    //encode to bytes
    uint32_t toBytes(char* bufferForMessage);

    std::queue<EventPair>&& giveEvents();

    size_t getEventsNumber();

    //can prepare events to be sent to GUI. Also changes client state
    //by reacting to events
    void prepareEvents(ClientState& clientState);
};

/*===========================================================================*
 *                        Enum: KeyStatus                                    *
 *===========================================================================*/

enum KeyStatus {
    LEFT_KEY_DOWN, LEFT_KEY_UP, RIGHT_KEY_DOWN, RIGHT_KEY_UP
};

/*===========================================================================*
 *                        Class: MessageFromGUI                              *
 *===========================================================================*/

//represents message from gui
class MessageFromGUI {
private:
    std::string message;
public:
    MessageFromGUI(char* encodedMessage, size_t messageSize);

    KeyStatus getStatus();
};

#endif //SIK_DUZE_MESSEGES_H
