#include <iostream>
#include <cstring>
#include "server_state.h"
#include "messeges.h"

/*===========================================================================*
 *                        Class: ServerPlayer                                *
 *===========================================================================*/

UDPMessenger& ServerPlayer::getMessenger() {
    return messenger;
}

bool ServerPlayer::operator<(const ServerPlayer& anotherPlayer) const {
    return *playerName < *anotherPlayer.playerName;
}

bool ServerPlayer::operator==(const ServerPlayer& anotherPlayer) const {
    return playerAddress.sin6_port == anotherPlayer.playerAddress.sin6_port &&
           !memcmp(
                   &playerAddress.sin6_addr,
                   &anotherPlayer.playerAddress.sin6_addr,
                   sizeof playerAddress.sin6_addr
           );
}

ServerPlayer::ServerPlayer(
        uint64_t newSessionID,
        std::shared_ptr<std::string> newPlayerName,
        sockaddr_in6& newPlayerAddress,
        int socket,
        char turnDirection
)
        :
        sessionID(newSessionID),
        playerName(newPlayerName),
        lastSeen(currentTime()),
        playerAddress(newPlayerAddress),
        messenger(socket, (sockaddr*) &playerAddress, sizeof playerAddress),
        lastDirection(turnDirection),
        isActive(turnDirection != 0) {
}

bool ServerPlayer::updateYourData(
        std::shared_ptr<ServerPlayer>& newPlayer, char turnDirection
) {
    if (!(
            playerAddress.sin6_port == newPlayer->playerAddress.sin6_port &&
            memcmp(
                    &playerAddress.sin6_addr,
                    &newPlayer->playerAddress.sin6_addr,
                    sizeof playerAddress.sin6_addr
            ) == 0
    )) {

        std::cerr
                << ntohs(playerAddress.sin6_port)
                << " "
                << ntohs(newPlayer->playerAddress.sin6_port);
        std::cerr << "Login already taken. Ignoring" << std::endl;
        return false;
    }

    if (newPlayer->sessionID >= sessionID) {
        sessionID = newPlayer->sessionID;
        lastSeen = currentTime();
        lastDirection = turnDirection;

        if (lastDirection != 0) {
            isActive = true;
        }
        return true;
    }
    else {
        std::cerr << "Smaller sessionID, ignoring" << std::endl;
        return false;
    }
}

size_t ServerPlayer::getNameSize() const {
    return playerName->length() + 1;
}

milliseconds ServerPlayer::getLastSeen() const {
    return lastSeen;
}

bool ServerPlayer::isPlayerActive() const {
    return isActive;
}

std::shared_ptr<std::string> ServerPlayer::getPlayerName() {
    return playerName;
}

char ServerPlayer::getLastTurningDirection() const {
    return lastDirection;
}

bool ServerPlayer::operator==(const sockaddr_in6& anotherSocket) const {
    return playerAddress.sin6_port == anotherSocket.sin6_port && !memcmp(
            &playerAddress.sin6_addr,
            &anotherSocket.sin6_addr,
            sizeof playerAddress.sin6_addr
    );
}

void ServerPlayer::becomeInactive() {
    isActive = false;
}

/*===========================================================================*
 *                        Struct: ComparePointers                            *
 *===========================================================================*/

bool ComparePointers::operator()(
        const std::shared_ptr<ServerPlayer>& player1,
        const std::shared_ptr<ServerPlayer>& player2
) const {
    return *player1 < *player2;
}

/*===========================================================================*
 *                        Struct: PairHash                                   *
 *===========================================================================*/


uint32_t PairHash::operator()(
        const std::pair<
                uint32_t, uint32_t
        >& pair
) const {
    return pair.first * 31 + pair.second;
}

/*===========================================================================*
 *                        Class: GameState                                   *
 *===========================================================================*/

static const size_t maxPlayersNicksLengthsSum = 512 - 25;
static const size_t maxPlayers = 42;

void GameState::addNewPlayer(std::shared_ptr<ServerPlayer>& player) {
    std::cout << "NEW PLAYER" << std::endl;
    if (activePlayers.size() + spectators.size() < maxPlayers && (
            player->getNameSize() == 1 ||
            playersNicksLengthSum + player->getNameSize() <=
            maxPlayersNicksLengthsSum
    )) {
        if (player->getNameSize() == 1) {
            spectators.push_back(player);
        }
        else {
            playersNicksLengthSum += player->getNameSize();
            activePlayers.insert(player);
        }
    }
    else {
        std::cerr << "Unfortunately, client cannot be served" << std::endl;
    }
}

void GameState::removeTimeoutedPlayers() {
    static const milliseconds timeout(2000);

    auto currentPlayer = activePlayers.begin();

    while (currentPlayer != activePlayers.end()) {
        if ((**currentPlayer).getLastSeen() + timeout < currentTime()) {
            activePlayers.erase(currentPlayer);
            playersNicksLengthSum -= (**currentPlayer).getNameSize();
        }
        ++currentPlayer;
    }

    auto currentSpectator = spectators.begin();
    for(size_t i = 0; i < spectators.size(); ++i) {
        if (spectators[i]->getLastSeen() + timeout < currentTime()) {
            spectators.erase(spectators.begin() + i);
        }
    }
}

size_t GameState::getActivePlayersNumber() {
    size_t activePlayersNumber = 0;

    for (auto& player: activePlayers) {
        activePlayersNumber += player->isPlayerActive();
    }

    return activePlayersNumber;
}

static const size_t maxMessageSize = 512;

void GameState::sendEvents(uint32_t number, UDPMessenger& partner) {
    static char bufferForMessage[maxMessageSize + 5] = {'\0'};

    if (number >= gameEvents.size()) {
        return;
    }

    while (number < gameEvents.size()) {
        std::queue<EventPair> eventPairs;
        size_t currentSize = 0;

        for (; number < gameEvents.size(); ++number) {
            if (gameEvents[number]->getSize() + currentSize > maxMessageSize) {
                break;
            }
            EventPair newPair = std::make_pair(gameEvents[number], number);
            eventPairs.push(std::move(newPair));
            currentSize += gameEvents[number]->getSize();
        }
        MessageFromServer newMessage(currentGameID, std::move(eventPairs));
        size_t size = newMessage.toBytes(bufferForMessage);
        partner.sendMessage(bufferForMessage, size);
    }
}
