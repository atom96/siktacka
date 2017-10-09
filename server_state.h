#ifndef SIK_DUZE_SERVER_STATE_H
#define SIK_DUZE_SERVER_STATE_H

#include <vector>
#include <set>
#include <unordered_set>
#include "events.h"
#include "udpmessenger.h"

using milliseconds = std::chrono::milliseconds;

class ServerPlayer;

class GameState;

/*===========================================================================*
 *                        Class: ServerPlayer                                *
 *===========================================================================*/

// represents client connected to server
class ServerPlayer {
    uint64_t sessionID;
    std::shared_ptr<std::string> playerName;
    milliseconds lastSeen;
    sockaddr_in6 playerAddress;
    UDPMessenger messenger;
    char lastDirection;
    bool isActive;

public:
    ServerPlayer(
            uint64_t newSessionID,
            std::shared_ptr<std::string> newPlayerName,
            sockaddr_in6& playerAddress,
            int socket,
            char turnDirection
    );

    UDPMessenger& getMessenger();

    bool operator==(const ServerPlayer& anotherPlayer) const;

    bool operator==(const sockaddr_in6& anotherSocket) const;

    bool operator<(const ServerPlayer& anotherPlayer) const;

    bool updateYourData(
            std::shared_ptr<ServerPlayer>& newPlayer, char turnDirection
    );

    size_t getNameSize() const;

    std::shared_ptr<std::string> getPlayerName();

    milliseconds getLastSeen() const;

    bool isPlayerActive() const;

    char getLastTurningDirection() const;

    void becomeInactive();
};

/*===========================================================================*
 *                        Class: PlayerHead                                  *
 *===========================================================================*/

//represents head of player's snake
class PlayerHead {
public:
    std::shared_ptr<ServerPlayer> player;
    double coordinateX;
    double coordinateY;
    int headRotation;
    bool isAlive;
};

/*===========================================================================*
 *                        Struct: ComparePointers                            *
 *===========================================================================*/
// Functor which comapares pointers to ServerPlayers by their name
struct ComparePointers {
    bool operator()(
            const std::shared_ptr<ServerPlayer>& player1,
            const std::shared_ptr<ServerPlayer>& player2
    ) const;
};

/*===========================================================================*
 *                        Struct: PairHash                                   *
 *===========================================================================*/
//Functor which hashes pair of uin32_t
struct PairHash {
    uint32_t operator()(const std::pair<uint32_t, uint32_t>& pair) const;
};

/*===========================================================================*
 *                        Class: GameState                                   *
 *===========================================================================*/

//represents current state of game
class GameState {
public:
    std::set<std::shared_ptr<ServerPlayer>, ComparePointers> activePlayers;
    std::vector<std::shared_ptr<ServerPlayer>> spectators;
    std::vector<PlayerHead> playerHeads;
    std::vector<std::shared_ptr<AbstractEvent>> gameEvents;
    std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash> takenFields;
    milliseconds lastTimeRoundPerformed;
    uint32_t currentGameID;
    size_t playersNicksLengthSum = 0;
    uint8_t alivePlayers = 0;
    int timeout = -1;
    int maxTimeout = -1;

public:
    //adds new player
    void addNewPlayer(std::shared_ptr<ServerPlayer>& player);

    //removes player which disconnected from server
    void removeTimeoutedPlayers();

    //returns number of players which pressed left or right key at least once
    size_t getActivePlayersNumber();

    //sends events from current game to partner
    void sendEvents(uint32_t number, UDPMessenger& partner);
};

#endif //SIK_DUZE_SERVER_STATE_H
