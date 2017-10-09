#include <iostream>
#include <cmath>
#include "server.h"
#include "messeges.h"

static const unsigned int clientIn = 0;
static const unsigned int clientOut = 1;
static const unsigned int pollSize = 2;

Server::Server(
        uint32_t width,
        uint32_t height,
        uint32_t speedOfGame,
        uint32_t speedOfTurning,
        UDPMessenger serverMessenger,
        RandomNumberGenerator randomNumberGenerator
)
        :
        boardWidth(width),
        boardHeight(height),
        gameSpeed(speedOfGame),
        turningSpeed(speedOfTurning),
        messenger(serverMessenger),
        random(randomNumberGenerator) {
}

void Server::preparePollTable(pollfd pollTable[]) {
    pollTable[clientIn].fd = messenger.getSocket();
    pollTable[clientIn].events = POLLIN;
    pollTable[clientIn].revents = 0;

    pollTable[clientOut].fd = messenger.getSocket();
    pollTable[clientOut].events = POLLOUT;
    pollTable[clientOut].revents = 0;
}

void Server::handleNewMessage(GameState& gameState) {
    static const unsigned int maxMessageFromPlayerLength = 8 + 1 + 4 + 64;

    static char messageBuffer[maxMessageFromPlayerLength + 5] = {'\0'};

    sockaddr_in6 clientSocket;
    socklen_t clientSize = sizeof clientSocket;
    size_t size = messenger.getMessageAndSenderData(
            messageBuffer,
            maxMessageFromPlayerLength + 1,
            (sockaddr*) &clientSocket,
            &clientSize
    );
    MessageToServer message(messageBuffer, size);

    auto newPlayer = std::shared_ptr<ServerPlayer>(
            new ServerPlayer(
                    message.getSessionID(),
                    message.getPlayerName(),
                    clientSocket,
                    messenger.getSocket(),
                    message.getDirection()));

    auto existingPlayerIterator = gameState.activePlayers.find(newPlayer);
    auto existingSpectatorIterator = gameState.spectators.end();

    for(int i = 0; i < gameState.spectators.size(); ++i) {
        if(*gameState.spectators[i] == *newPlayer) {
            existingSpectatorIterator = gameState.spectators.begin() + i;
        }
    }

    if (existingPlayerIterator != gameState.activePlayers.end()) {
        auto& existingPlayer = const_cast<std::shared_ptr<
                ServerPlayer
        >&>(*existingPlayerIterator);
        if (existingPlayer->updateYourData(newPlayer, message.getDirection())) {
            gameState.sendEvents(
                    message.getExpectedEventNumber(),
                    existingPlayer->getMessenger());
        }
    }
    else if (existingSpectatorIterator != gameState.spectators.end()) {
        auto& existingPlayer = const_cast<std::shared_ptr<
                ServerPlayer
        >&>(*existingSpectatorIterator);
        if (existingPlayer->updateYourData(newPlayer, message.getDirection())) {

            gameState.sendEvents(
                    message.getExpectedEventNumber(),
                    existingPlayer->getMessenger());
        }
    }
    else {
        gameState.addNewPlayer(newPlayer);
        gameState.sendEvents(
                message.getExpectedEventNumber(), newPlayer->getMessenger());
    }
}

bool Server::isFieldTaken(
        GameState& gameState, double coordinateX, double coordinateY
) {
    return (
                   gameState.takenFields.find(
                           std::make_pair((uint32_t) floor(coordinateX),
                                          (uint32_t) floor(coordinateY))) !=
                   gameState.takenFields.end()) ||
           coordinateX >= boardWidth ||
           coordinateX < 0 ||
           coordinateY >= boardHeight ||
           coordinateY < 0;
}

void Server::createNewGame(GameState& gameState) {
    std::cout << "NEW GAME" << std::endl;

    gameState.gameEvents.clear();
    gameState.currentGameID = random.random32BitInteger();
    gameState.maxTimeout = 1000 / gameSpeed;
    gameState.timeout = gameState.maxTimeout;
    gameState.lastTimeRoundPerformed = currentTime();
    gameState.alivePlayers = 0;
    gameState.gameEvents.clear();
    gameState.playerHeads.clear();
    gameState.takenFields.clear();
    std::vector<std::string> playerNames;

    for (auto& player : gameState.activePlayers) {
        playerNames.push_back(*player->getPlayerName());
    }

    std::shared_ptr<AbstractEvent>
            newGameEvent = std::shared_ptr<AbstractEvent>(
            new NewGameEvent(
                    boardWidth, boardHeight, std::move(playerNames)));

    gameState.gameEvents.push_back(newGameEvent);

    uint8_t i = 0;
    for (auto& player : gameState.activePlayers) {
        PlayerHead newHead;
        newHead.player = player;
        newHead.coordinateX = (random.random32BitInteger() % boardWidth) + 0.5;
        newHead.coordinateY = (random.random32BitInteger() % boardHeight) + 0.5;
        newHead.headRotation = random.random32BitInteger() % 360;
        newHead.isAlive = true;

        if (!isFieldTaken(
                gameState, newHead.coordinateX, newHead.coordinateY
        )) {

            std::shared_ptr<AbstractEvent>
                    newPixelEvent = std::shared_ptr<AbstractEvent>(
                    new PixelEvent(
                            i,
                            (uint32_t) newHead.coordinateX,
                            (uint32_t) newHead.coordinateY
                    ));

            gameState.takenFields.insert(
                    std::make_pair((uint32_t) floor(newHead.coordinateX),
                                   (uint32_t) floor(newHead.coordinateY)));
            gameState.alivePlayers++;
        }
        else {
            std::shared_ptr<AbstractEvent>
                    newPixelEvent = std::shared_ptr<AbstractEvent>(
                    new PlayerEliminatedEvent(i));
            newHead.isAlive = false;
        }
        gameState.playerHeads.push_back(std::move(newHead));
        ++i;
    }
}

void Server::performRound(GameState& gameState) {
    uint8_t i = 0;
    gameState.lastTimeRoundPerformed = currentTime();

    for (auto& playerHead: gameState.playerHeads) {
        if (!playerHead.isAlive) {
            ++i;
            continue;
        }

        switch (playerHead.player->getLastTurningDirection()) {
            case 1:
                playerHead.headRotation += turningSpeed;
                playerHead.headRotation %= 360;
                break;
            case -1:
                playerHead.headRotation -= turningSpeed;
                playerHead.headRotation += 360;
                playerHead.headRotation %= 360;
                break;
            default:
                break;
        }

        double newCoordinateX = playerHead.coordinateX + cos(
                static_cast<double>(playerHead.headRotation) * M_PI / 180
        );

        double newCoordinateY = playerHead.coordinateY + sin(
                static_cast<double>(playerHead.headRotation) * M_PI / 180
        );

        if (floor(newCoordinateX) == floor(playerHead.coordinateX) &&
            floor(newCoordinateY) == floor(playerHead.coordinateY)) {
            playerHead.coordinateX = newCoordinateX;
            playerHead.coordinateY = newCoordinateY;
            ++i;
            continue;
        }

        if (isFieldTaken(gameState, newCoordinateX, newCoordinateY)) {
            gameState.gameEvents.push_back(
                    std::shared_ptr<AbstractEvent>(
                            new PlayerEliminatedEvent(i)));
            gameState.alivePlayers--;
        }
        else {
            gameState.gameEvents.push_back(
                    std::shared_ptr<AbstractEvent>(
                            new PixelEvent(
                                    i,
                                    (uint32_t) floor(newCoordinateX),
                                    (uint32_t) floor(newCoordinateY))));
            gameState.takenFields.insert(
                    std::make_pair(
                            floor(newCoordinateX), floor(newCoordinateY)));
            playerHead.coordinateX = newCoordinateX;
            playerHead.coordinateY = newCoordinateY;
        }

        if (gameState.alivePlayers <= 1) {
            gameState.gameEvents.push_back(
                    std::shared_ptr<AbstractEvent>(new GameOverEvent()));
            for (auto& player : gameState.activePlayers) {
                auto& mutablePlayer =
                        const_cast<std::shared_ptr<ServerPlayer>&>(player);
                mutablePlayer->becomeInactive();
            }
            gameState.timeout = -1;
            gameState.maxTimeout = -1;
            return;
        }

        ++i;
    }
}

void Server::startListening() {
    GameState gameState;
    pollfd pollTable[pollSize];

    while (true) {
        preparePollTable(pollTable);
        int ret = poll(pollTable, pollSize, gameState.timeout);
        try {
            if (ret == -1) {
                std::cerr
                        << "Error with poll."
                        << " Server is not able to continue to live"
                        << std::endl;
                exit(ERROR);
            }

            auto xd = gameState.lastTimeRoundPerformed +
                      milliseconds(gameState.maxTimeout) -
                      currentTime();
            if (gameState.maxTimeout != -1 &&
                milliseconds(gameState.maxTimeout) <
                currentTime() - gameState.lastTimeRoundPerformed) {
                performRound(gameState);
            }

            if (pollTable[clientIn].revents & POLLIN &&
                pollTable[clientOut].revents & POLLOUT) {

                handleNewMessage(gameState);
            }

            if (gameState.maxTimeout == -1 &&
                gameState.activePlayers.size() > 1 &&
                gameState.getActivePlayersNumber() ==
                gameState.activePlayers.size()) {
                std::cout << "NEW GAME" << std::endl;
                createNewGame(gameState);
            }

            std::cout << gameState.activePlayers.size() << " " << gameState.getActivePlayersNumber() << std::endl;

            gameState.removeTimeoutedPlayers();
            if (gameState.maxTimeout != -1) {
                auto diff = currentTime() - gameState.lastTimeRoundPerformed;

                long diff2 = gameState.maxTimeout - diff.count();

                gameState.timeout = static_cast<int>(diff2);
            }
        }
        catch (std::exception& e) {
            std::cerr << "Message ignored " << e.what() << std::endl;
            continue;
        }
        catch (...) {
            std::cerr
                    << "Something really unexpected happened, "
                    << "but server will continue to live anyway"
                    << std::endl;
        }
    }
}