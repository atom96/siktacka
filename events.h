//
// Created by arek on 02.06.17.
//

#ifndef SIK_DUZE_EVENTS_H
#define SIK_DUZE_EVENTS_H

#include <vector>
#include "helper.h"
#include "client_state.h"

// These classes represent event which may be send between client and server

/*===========================================================================*
 *                        Class: AbstractEvent                               *
 *===========================================================================*/

class AbstractEvent {
protected:
    AbstractEvent() {
    };
public:
    // encodes event to byte representation
    virtual uint32_t toBytes(char* bufferForMessage) = 0;

    // encodes event to string, so it can be sent to GUI
    virtual std::string toGUIMessage(std::vector<std::string>& players) = 0;

    virtual char giveType() = 0;

    // changes client state. Returns true if error checking was positive
    // false otherwise. Guaranties not to change state if false was returned
    virtual bool changeClientState(ClientState& clientState, uint32_t myNumber);

    // returns size of event in bytes
    virtual size_t getSize() = 0;
};

/*===========================================================================*
 *                        Class: NewGameEvent                                *
 *===========================================================================*/

class NewGameEvent : public AbstractEvent {
private:
    uint32_t width;
    uint32_t height;
    std::vector<std::string> players;
    static const char typeID = 0;

private:
    void parseWidth(const char*& encodedEvent, size_t& dataSize);

    void parseHeight(const char*& encodedEvent, size_t& dataSize);

    void parsePlayer(const char*& encodedEvent, size_t& dataSize);

    void parsePlayers(const char*& encodedEvent, size_t& dataSize);

    void encodeWidth(char*& bufferForMessage, uint32_t& totalSize);

    void encodeHeight(char*& bufferForMessage, uint32_t& totalSize);

    void encodePlayers(char*& bufferForMessage, uint32_t& totalSize);

    void encodePlayer(char*& bufferForMessage, uint32_t& totalSize,
                      std::string& player);

    void encodeType(char*& bufferForMessage, uint32_t& totalSize);

public:
    NewGameEvent(uint32_t boardWidth, uint32_t boardHeight,
                 std::vector<std::string>&& newPlayers);

    NewGameEvent(const char* encodedEvent, size_t dataSize);

    uint32_t toBytes(char* bufferForMessage);

    std::vector<std::string>&& givePlayersBack();

    operator std::string() const;

    bool operator==(const NewGameEvent& nge) const;

    std::string toGUIMessage(std::vector<std::string>& players);

    bool changeClientState(ClientState& clientState, uint32_t myNumber);

    char giveType();

    size_t getSize();
};

/*===========================================================================*
 *                        Class: PixelEvent                                  *
 *===========================================================================*/

class PixelEvent : public AbstractEvent {
private:
    static const char typeID = 1;
    uint8_t playerNumber;
    uint32_t coordinateX;
    uint32_t coordinateY;

    void parsePlayerNumber(const char*& encodedEvent, size_t& dataSize);

    void parseCoordinateX(const char*& encodedEvent, size_t& dataSize);

    void parseCoordinateY(const char*& encodedEvent, size_t& dataSize);

    void encodeType(char*& bufferForMessage, uint32_t& totalLength);

    void encodePlayerNumber(char*& bufferForMessage, uint32_t& totalLength);

    void encodeCoordinateX(char*& bufferForMessage, uint32_t& totalLength);

    void encodeCoordinateY(char*& bufferForMessage, uint32_t& totalLength);

public:
    PixelEvent(uint8_t newPlayerNumber, uint32_t newCoordinateX,
               uint32_t newCoordinateY);

    PixelEvent(const char* encodedEvent, size_t dataSize);

    uint32_t toBytes(char* bufferForMessage);

    bool operator==(const PixelEvent& pe) const;

    operator std::string() const;

    std::string toGUIMessage(std::vector<std::string>& players);

    char giveType();

    bool changeClientState(ClientState& clientState, uint32_t myNumber);

    size_t getSize();
};

/*===========================================================================*
 *                        Class: PlayerEliminatedEvent                       *
 *===========================================================================*/

class PlayerEliminatedEvent : public AbstractEvent {
private:
    uint8_t playerNumber;
    static const char typeID = 2;

    void parsePlayerNumber(const char*& encodedEvent, size_t& dataSize);

    void encodeType(char*& bufferForMessage, uint32_t& totalSize);

    void encodePlayerNumber(char*& bufferForMessage, uint32_t& totalSize);

public:
    PlayerEliminatedEvent(uint8_t newPlayerNumber);

    PlayerEliminatedEvent(const char* encodedEvent, size_t dataSize);

    uint32_t toBytes(char* bufferForMessage);

    bool operator==(const PlayerEliminatedEvent& pee) const;

    std::string toGUIMessage(std::vector<std::string>& players);

    char giveType();

    bool changeClientState(ClientState& clientState, uint32_t myNumber);

    size_t getSize();
};

/*===========================================================================*
 *                        Class: GameOverEvent                               *
 *===========================================================================*/

class GameOverEvent : public AbstractEvent {
private:
    static const char typeID = 3;

    void encodeType(char*& bufferForMessage, uint32_t& totalSize);

public:
    GameOverEvent();

    GameOverEvent(const char* encodedEvent, size_t dataSize);

    uint32_t toBytes(char* bufferForMessage);

    bool operator==(const GameOverEvent& goe) const;

    std::string toGUIMessage(std::vector<std::string>& players);

    char giveType();

    bool changeClientState(ClientState& clientState, uint32_t myNumber);

    size_t getSize();
};

#endif //SIK_DUZE_EVENTS_H
