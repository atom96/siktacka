#include <netdb.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <unordered_map>
#include "helper.h"
#include "udpmessenger.h"
#include "server.h"

int main(int argc, char** argv) {
    uint32_t boardWidth = 800;
    uint32_t boardHeight = 600;
    uint16_t port = 12345;
    uint32_t gameSpeed = 50;
    uint32_t turningSpeed = 6;
    int64_t seed = time(NULL);

    //parsing arguments. If they are invalid or passed twice server will die
    std::unordered_map<char, bool> doneFields;
    doneFields['W'] = false;
    doneFields['H'] = false;
    doneFields['p'] = false;
    doneFields['s'] = false;
    doneFields['t'] = false;
    doneFields['r'] = false;

    for (int i = 1; i < argc - 1; i += 2, argc -= 2) {
        std::istringstream ss(std::string(argv[i + 1]));
        char* value = argv[i + 1];

        while (*value != '\0') {
            if (!isdigit(*value)) {
                std::cerr << "Value contains illegal characters" << std::endl;
                exit(ERROR);
            }
            ++value;
        }

        if (strcmp(argv[i], "-W") == 0) {
            if (doneFields['W']) {
                std::cerr << "Same parameter sent more than once" << std::endl;
                exit(ERROR);
            }
            ss >> boardWidth;
            doneFields['W'] = true;
        }
        else if (strcmp(argv[i], "-H") == 0) {
            if (doneFields['H']) {
                std::cerr << "Same parameter sent more than once" << std::endl;
                exit(ERROR);
            }
            ss >> boardHeight;
            doneFields['H'] = true;
        }
        else if (strcmp(argv[i], "-p") == 0) {
            if (doneFields['p']) {
                std::cerr << "Same parameter sent more than once" << std::endl;
                exit(ERROR);
            }
            ss >> port;
            doneFields['p'] = true;
        }
        else if (strcmp(argv[i], "-s") == 0) {
            if (doneFields['s']) {
                std::cerr << "Same parameter sent more than once" << std::endl;
                exit(ERROR);
            }
            ss >> gameSpeed;
            doneFields['s'] = true;
        }
        else if (strcmp(argv[i], "-t") == 0) {
            if (doneFields['t']) {
                std::cerr << "Same parameter sent more than once" << std::endl;
                exit(ERROR);
            }
            ss >> turningSpeed;
            doneFields['t'] = true;
        }
        else if (strcmp(argv[i], "-r") == 0) {
            if (doneFields['r']) {
                std::cerr << "Same parameter sent more than once" << std::endl;
                exit(ERROR);
            }
            ss >> seed;
            doneFields['r'] = true;
        }
        else {
            std::cerr << "Invalid argument " << argv[i] << std::endl;
            exit(ERROR);
        }

        if (ss.fail() || !ss.eof()) {
            std::cerr
                    << "Invalid value of parameter "
                    << argv[i]
                    << argv[i + 1]
                    << std::endl;
            exit(ERROR);
        }
    }

    if (argc == 2) {
        std::cout << "Invalid parameter " << std::endl;
        exit(ERROR);
    }

    //creating socket for server
    int serverSocket = socket(AF_INET6, SOCK_DGRAM, 0);

    if (serverSocket < 0) {
        std::cerr << "error while gaining socket" << std::endl;
    }

    //filling addres info with basic information
    sockaddr_in6 myAddress;
    myAddress.sin6_family = AF_INET6;
    myAddress.sin6_addr = in6addr_any;
    myAddress.sin6_flowinfo = 0;
    myAddress.sin6_port = htons(port);

    //binding socket
    if (bind(serverSocket, (struct sockaddr*) &myAddress, sizeof myAddress) <
        0) {
        std::cerr << "Error while binding the socket" << std::endl;
        exit(ERROR);
    }

    UDPMessenger
            messenger(serverSocket, (sockaddr*) &myAddress, sizeof myAddress);
    RandomNumberGenerator random((uint64_t) seed);

    Server server(
            boardWidth, boardHeight, gameSpeed, turningSpeed, messenger, random
    );

    //executing main loop
    server.startListening();

    return 0;
}