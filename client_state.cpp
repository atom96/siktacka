#include "client_state.h"

ClientState::ClientState()
        :
        isLeftKeyPressed(false),
        isRightKeyPressed(false),
        wasRightKeyPressedLast(false),
        sessionID(static_cast<uint64_t>(time(NULL))),
        nextExpectedEventNumber(0),
        gameID(0),
        expectedGameID(1),
        boardWidth(0),
        boardHeight(0) {
}
