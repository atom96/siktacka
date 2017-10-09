//
// Created by arek on 02.06.17.
//
#include <cstring>
#include <iostream>
#include "events.h"

static const size_t defaultDataSize = 4 + 4 + 1 + 4;

//in normal game we receive A LOT OF old messages, so reporting every bug
//about them is really annoying. However, we can turn it on anytime
static const bool showEventsErrors = false;

/*===========================================================================*
 *                        Class: AbstractEvent                               *
 *===========================================================================*/

bool AbstractEvent::changeClientState(
        ClientState& clientState, uint32_t myNumber
) {
    if (clientState.nextExpectedEventNumber == 0) {
        if (showEventsErrors) {
            std::cerr
                    << "Its not NEW GAME. NEW GAME event should be first event."
                    << " Ignoring event. Type: "
                    << (int) giveType()
                    << " Number: "
                    << myNumber
                    << std::endl;
        }
        return false;
    }
    if (myNumber == clientState.nextExpectedEventNumber) {
        clientState.nextExpectedEventNumber++;
        return true;
    }
    else {
        if (myNumber > clientState.nextExpectedEventNumber) {
            std::cerr
                    << "Event of type: "
                    << numberToString((int) giveType())
                    << "has invalid number. Expected: "
                    << numberToString(clientState.nextExpectedEventNumber)
                    << " got: "
                    << numberToString(myNumber)
                    << std::endl;
        }
        return false;
    }
}

/*===========================================================================*
 *                        Class: NewGameEvent                                *
 *===========================================================================*/
NewGameEvent::NewGameEvent(
        uint32_t boardWidth,
        uint32_t boardHeight,
        std::vector<std::string>&& newPlayers
) : width(boardWidth), height(boardHeight), players(newPlayers) {
    std::cout << "NEW GAME" << std::endl;
}

void NewGameEvent::parseWidth(const char*& encodedEvent, size_t& dataSize) {
    if (dataSize < sizeof width) {
        throw MessageSizeException(
                "Message too short to contain width " +
                numberToString(dataSize));
    }
    width = be32toh(*(uint32_t*) encodedEvent);
    encodedEvent += sizeof width;
    dataSize -= sizeof width;
}

void NewGameEvent::parseHeight(const char*& encodedEvent, size_t& dataSize) {
    if (dataSize < sizeof height) {
        throw MessageSizeException(
                "Message too short to contain height: " +
                numberToString(dataSize));
    }

    height = be32toh(*(uint32_t*) encodedEvent);
    encodedEvent += sizeof height;
    dataSize -= sizeof height;
}

void NewGameEvent::parsePlayer(const char*& encodedEvent, size_t& dataSize) {
    std::string player(encodedEvent);

    dataSize -= player.length() + 1;
    encodedEvent += player.length() + 1;

    if (player.length() > maxNameLength) {
        throw InvalidArgumentException("Player name too long");
    }

    players.push_back(player);
}

void NewGameEvent::parsePlayers(const char*& encodedEvent, size_t& dataSize) {
    if (encodedEvent[dataSize - 1] != '\0') {
        throw InvalidArgumentException(
                "Data should be ended with NULL character"
        );
    }

    while (dataSize > 0) {
        parsePlayer(encodedEvent, dataSize);
    }

    if (dataSize != 0) {
        throw UnexpectedSituationException(
                "Read more than data size which never should have happened"
        );
    }
}

NewGameEvent::NewGameEvent(const char* encodedEvent, size_t dataSize) {
    if (encodedEvent == nullptr) {
        throw NullPointerException("Buffer cannot be NULL");
    }

    parseWidth(encodedEvent, dataSize);
    parseHeight(encodedEvent, dataSize);
    parsePlayers(encodedEvent, dataSize);
}

void NewGameEvent::encodeWidth(char*& bufferForMessage, uint32_t& totalSize) {
    uint32_t encodedWidth = htobe32(width);
    memcpy((void*) bufferForMessage, (void*) &encodedWidth, sizeof encodedWidth
    );
    bufferForMessage += sizeof encodedWidth;
    totalSize += sizeof encodedWidth;
}

void NewGameEvent::encodeHeight(char*& bufferForMessage, uint32_t& totalSize) {
    uint32_t encodedHeight = htobe32(height);
    memcpy((void*) bufferForMessage,
           (void*) &encodedHeight,
           sizeof encodedHeight
    );
    bufferForMessage += sizeof encodedHeight;
    totalSize += sizeof encodedHeight;
}

void NewGameEvent::encodePlayer(
        char*& bufferForMessage, uint32_t& totalSize, std::string& player
) {
    memcpy((void*) bufferForMessage, (void*) player.c_str(), player.length() + 1
    );
    bufferForMessage += player.length() + 1;
    totalSize += player.length() + 1;
}

void NewGameEvent::encodePlayers(char*& bufferForMessage, uint32_t& totalSize) {
    for (std::string& player: players) {
        encodePlayer(bufferForMessage, totalSize, player);
    }
}

void NewGameEvent::encodeType(char*& bufferForMessage, uint32_t& totalSize) {
    *bufferForMessage = typeID;
    ++bufferForMessage;
    ++totalSize;
}

uint32_t NewGameEvent::toBytes(char* bufferForMessage) {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Cannot write to NULL buffer");
    }

    uint32_t totalSize = 0;

    encodeType(bufferForMessage, totalSize);
    encodeWidth(bufferForMessage, totalSize);
    encodeHeight(bufferForMessage, totalSize);
    encodePlayers(bufferForMessage, totalSize);

    return totalSize;
}

std::vector<std::string>&& NewGameEvent::givePlayersBack() {
    return std::move(players);
}

NewGameEvent::operator std::string() const {
    std::string playersString = "";
    for (auto player : players) {
        playersString += player;
        playersString += " ";
    }

    return numberToString(width) +
           " " +
           numberToString(height) +
           " " +
           playersString +
           numberToString(players.size());
}

bool NewGameEvent::operator==(const NewGameEvent& nge) const {
    return width == nge.width && height == nge.height && players == nge.players;
}

std::string NewGameEvent::toGUIMessage(std::vector<std::string>& players) {
    std::string playersString = "NEW_GAME " +
                                numberToString(width) +
                                " " +
                                numberToString(height) +
                                " ";

    for (auto player : players) {
        playersString += player;
        playersString += " ";
    }

    playersString.pop_back();
    playersString += "\n";

    return playersString;
}

char NewGameEvent::giveType() {
    return typeID;
}

bool NewGameEvent::changeClientState(
        ClientState& clientState, uint32_t myNumber
) {
    if (myNumber == 0 &&
        clientState.nextExpectedEventNumber == 0 &&
        clientState.expectedGameID != clientState.gameID) {
        clientState.nextExpectedEventNumber++;
        clientState.messegesToSend.push(toGUIMessage(players));
        clientState.playerNames = std::move(players);
        clientState.gameID = clientState.expectedGameID;
        clientState.boardHeight = height;
        clientState.boardWidth = width;
        return true;
    }
    else {
        if (clientState.expectedGameID == clientState.gameID) {
            if (showEventsErrors) {
                std::cerr
                        << "Received old game ID"
                        << clientState.gameID
                        << std::endl;
            }
            return false;
        }

        if (showEventsErrors) {
            std::cerr
                    << "NEW GAME always should be firs event. Got "
                    << numberToString(myNumber)
                    << " expected "
                    << numberToString(clientState.nextExpectedEventNumber)
                    << "Ignored"
                    << std::endl;
        }
        return false;
    }
}

size_t NewGameEvent::getSize() {
    size_t mySize = sizeof width + sizeof height;

    for (auto& player: players) {
        mySize += player.length() + 1;
    }

    return defaultDataSize;
}

/*===========================================================================*
 *                        Class: PixelEvent                                  *
 *===========================================================================*/

PixelEvent::PixelEvent(
        uint8_t newPlayerNumber,
        uint32_t newCoordinateX,
        uint32_t newCoordinateY
)
        :
        playerNumber(newPlayerNumber),
        coordinateX(newCoordinateX),
        coordinateY(newCoordinateY) {
}

void PixelEvent::parsePlayerNumber(
        const char*& encodedEvent, size_t& dataSize
) {
    if (dataSize < sizeof playerNumber) {
        throw MessageSizeException(
                "Message too short to contain player number: " +
                numberToString(dataSize));
    }
    playerNumber = *encodedEvent;
    ++encodedEvent;
    --dataSize;
}

void PixelEvent::parseCoordinateX(const char*& encodedEvent, size_t& dataSize) {
    if (dataSize < sizeof coordinateX) {
        throw MessageSizeException(
                "Message too short to contain coordinate X: " +
                numberToString(dataSize));
    }
    coordinateX = be32toh(*(uint32_t*) encodedEvent);
    encodedEvent += sizeof coordinateX;
    dataSize -= sizeof coordinateX;
}

void PixelEvent::parseCoordinateY(const char*& encodedEvent, size_t& dataSize) {
    if (dataSize < sizeof coordinateY) {
        throw MessageSizeException(
                "Message too short to contain coordinate Y: " +
                numberToString(dataSize));
    }
    coordinateY = be32toh(*(uint32_t*) encodedEvent);
    encodedEvent += sizeof coordinateY;
    dataSize -= sizeof coordinateY;
}

PixelEvent::PixelEvent(const char* encodedEvent, size_t dataSize) {
    if (encodedEvent == nullptr) {
        throw NullPointerException("Buffer cannot be NULL");
    }

    parsePlayerNumber(encodedEvent, dataSize);
    parseCoordinateX(encodedEvent, dataSize);
    parseCoordinateY(encodedEvent, dataSize);

    if (dataSize != 0) {
        throw MessageSizeException(
                "Message too long for PixelEvent. Still left: " +
                numberToString(dataSize));
    }
}

void PixelEvent::encodeType(char*& bufferForMessage, uint32_t& totalLength) {
    *bufferForMessage = typeID;
    ++bufferForMessage;
    ++totalLength;
}

void PixelEvent::encodePlayerNumber(
        char*& bufferForMessage, uint32_t& totalLength
) {
    *bufferForMessage = playerNumber;
    ++bufferForMessage;
    ++totalLength;
}

void PixelEvent::encodeCoordinateX(
        char*& bufferForMessage, uint32_t& totalLength
) {
    uint32_t encodedCoordinateX = htobe32(coordinateX);
    memcpy((void*) bufferForMessage,
           (void*) &encodedCoordinateX,
           sizeof encodedCoordinateX
    );
    bufferForMessage += sizeof encodedCoordinateX;
    totalLength += sizeof encodedCoordinateX;
}

void PixelEvent::encodeCoordinateY(
        char*& bufferForMessage, uint32_t& totalLength
) {
    uint32_t encodedCoordinateY = htobe32(coordinateY);
    memcpy((void*) bufferForMessage,
           (void*) &encodedCoordinateY,
           sizeof encodedCoordinateY
    );
    bufferForMessage += sizeof encodedCoordinateY;
    totalLength += sizeof encodedCoordinateY;
}

uint32_t PixelEvent::toBytes(char* bufferForMessage) {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Cannot write to NULL buffer");
    }

    uint32_t totalLength = 0;

    encodeType(bufferForMessage, totalLength);
    encodePlayerNumber(bufferForMessage, totalLength);
    encodeCoordinateX(bufferForMessage, totalLength);
    encodeCoordinateY(bufferForMessage, totalLength);

    return totalLength;
}

bool PixelEvent::operator==(const PixelEvent& pe) const {
    return playerNumber == pe.playerNumber &&
           coordinateY == pe.coordinateY &&
           coordinateX == pe.coordinateX;
}

PixelEvent::operator std::string() const {
    return numberToString(playerNumber) +
           " " +
           numberToString(coordinateX) +
           " " +
           numberToString(coordinateY);
}

std::string PixelEvent::toGUIMessage(std::vector<std::string>& players) {
    if (players.size() < playerNumber) {
        throw InvalidArgumentException(
                "Size is: " +
                numberToString(players.size()) +
                " and cannot contain " +
                numberToString(playerNumber));
    }

    return "PIXEL " +
           numberToString(coordinateX) +
           " " +
           numberToString(coordinateY) +
           " " +
           players[playerNumber] +
           "\n";
}

char PixelEvent::giveType() {
    return typeID;
}

bool PixelEvent::changeClientState(
        ClientState& clientState, uint32_t myNumber
) {
    if (AbstractEvent::changeClientState(clientState, myNumber)) {
        if (coordinateX < clientState.boardWidth &&
            coordinateY < clientState.boardHeight) {
            clientState.messegesToSend
                       .push(toGUIMessage(clientState.playerNames));
            return true;
        }
        if (showEventsErrors) {
            std::cerr << "Wrong coordinates" << std::endl;
        }
        return false;
    }
    else {
        return false;
    }
}

size_t PixelEvent::getSize() {
    return defaultDataSize +
           sizeof playerNumber +
           sizeof coordinateX +
           sizeof coordinateY;
}

/*===========================================================================*
 *                        Class: PlayerEliminatedEvent                       *
 *===========================================================================*/

PlayerEliminatedEvent::PlayerEliminatedEvent(uint8_t newPlayerNumber)
        :
        playerNumber(
                newPlayerNumber
        ) {
}

void PlayerEliminatedEvent::parsePlayerNumber(
        const char*& encodedEvent, size_t& dataSize
) {
    if (dataSize < sizeof playerNumber) {
        throw MessageSizeException(
                "Too short message to contain playerNumber: " +
                numberToString(dataSize));
    }

    playerNumber = *encodedEvent;
    ++encodedEvent;
    dataSize -= sizeof playerNumber;
}

PlayerEliminatedEvent::PlayerEliminatedEvent(
        const char* encodedEvent, size_t dataSize
) {
    if (encodedEvent == nullptr) {
        throw NullPointerException("Buffer cannot be NULL");
    }

    parsePlayerNumber(encodedEvent, dataSize);

    if (dataSize != 0) {
        throw MessageSizeException(
                "Message too long. Still got: " + numberToString(dataSize));
    }
}

void PlayerEliminatedEvent::encodeType(
        char*& bufferForMessage, uint32_t& totalSize
) {
    *bufferForMessage = typeID;
    ++bufferForMessage;
    ++totalSize;
}

void PlayerEliminatedEvent::encodePlayerNumber(
        char*& bufferForMessage, uint32_t& totalSize
) {
    *bufferForMessage = playerNumber;
    ++bufferForMessage;
    ++totalSize;
}

uint32_t PlayerEliminatedEvent::toBytes(char* bufferForMessage) {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Cannot write to NULL buffer");
    }

    uint32_t totalSize = 0;
    encodeType(bufferForMessage, totalSize);
    encodePlayerNumber(bufferForMessage, totalSize);

    return totalSize;
}

bool PlayerEliminatedEvent::operator==(const PlayerEliminatedEvent& pee) const {
    return playerNumber == pee.playerNumber;
}

std::string PlayerEliminatedEvent::toGUIMessage(std::vector<std::string>& players) {
    if (players.size() <= playerNumber) {
        throw InvalidArgumentException(
                "Player list does not contain this player"
        );
    }
    return "PLAYER_ELIMINATED " + players[playerNumber] + "\n";
}

char PlayerEliminatedEvent::giveType() {
    return typeID;
}

bool PlayerEliminatedEvent::changeClientState(
        ClientState& clientState, uint32_t myNumber
) {
    if (AbstractEvent::changeClientState(clientState, myNumber)) {
        clientState.messegesToSend.push(toGUIMessage(clientState.playerNames));
        return true;
    }
    else {
        return false;
    }
}

size_t PlayerEliminatedEvent::getSize() {
    return defaultDataSize + sizeof playerNumber;
}

/*===========================================================================*
 *                        Class: GameOverEvent                               *
 *===========================================================================*/

GameOverEvent::GameOverEvent() {
}

GameOverEvent::GameOverEvent(const char* encodedEvent, size_t dataSize) {
    if (encodedEvent == nullptr) {
        throw NullPointerException("Buffer cannot be NULL");
    }

    if (dataSize != 0) {
        throw InvalidArgumentException(
                "Wrong data size : " + numberToString(dataSize));
    }
}

void GameOverEvent::encodeType(char*& bufferForMessage, uint32_t& totalSize) {
    *bufferForMessage = typeID;
    ++bufferForMessage;
    ++totalSize;
}

uint32_t GameOverEvent::toBytes(char* bufferForMessage) {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Cannot write to NULL buffer");
    }
    uint32_t totalSize = 0;

    encodeType(bufferForMessage, totalSize);

    return totalSize;
}

bool GameOverEvent::operator==(const GameOverEvent& goe) const {
    return true;
}

std::string GameOverEvent::toGUIMessage(std::vector<std::string>& players) {
    throw UnexpectedSituationException("Game over event cannot be sent to GUI");
}

char GameOverEvent::giveType() {
    return typeID;
}

bool GameOverEvent::changeClientState(
        ClientState& clientState, uint32_t myNumber
) {
    if (AbstractEvent::changeClientState(clientState, myNumber)) {
        clientState.nextExpectedEventNumber = 0;
        return true;
    }

    return false;
}

size_t GameOverEvent::getSize() {
    return defaultDataSize;
}