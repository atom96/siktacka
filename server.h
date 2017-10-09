#ifndef SIK_DUZE_SERVER_H
#define SIK_DUZE_SERVER_H

#include <cstdint>
#include <poll.h>
#include "helper.h"
#include "udpmessenger.h"
#include "server_state.h"

//class which represents game server. Is responsible for communication
//with clients and performing games
class Server {
    uint32_t boardWidth;
    uint32_t boardHeight;
    uint32_t gameSpeed;
    uint32_t turningSpeed;
    UDPMessenger messenger;
    RandomNumberGenerator random;
private:
    void preparePollTable(pollfd pollTable[]);

    void handleNewMessage(GameState& gameState);

    void createNewGame(GameState& gameState);

    void performRound(GameState& gameState);

    bool isFieldTaken(
            GameState& gameState, double coordinateX, double coordinateY
    );

public:
    Server(
            uint32_t width,
            uint32_t height,
            uint32_t speedOfGame,
            uint32_t speedOfTurning,
            UDPMessenger serverMessenger,
            RandomNumberGenerator randomNumberGenerator
    );

    void startListening();
};

#endif //SIK_DUZE_SERVER_H
