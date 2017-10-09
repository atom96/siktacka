//
// Created by arek on 06.06.17.
//

#ifndef SIK_DUZE_CLIENT_STATE_H
#define SIK_DUZE_CLIENT_STATE_H

#include <cstdint>
#include <memory>
#include <queue>


// Class which represents current client state
class ClientState {
public:
    bool isLeftKeyPressed;
    bool isRightKeyPressed;
    bool wasRightKeyPressedLast;
    std::queue<std::string> messegesToSend;
    uint64_t sessionID;
    uint32_t nextExpectedEventNumber;
    uint32_t gameID;
    uint32_t expectedGameID;
    std::vector<std::string> playerNames;
    uint32_t boardWidth;
    uint32_t boardHeight;

    ClientState();
};

#endif //SIK_DUZE_CLIENT_STATE_H
