#include <iostream>
#include <cstring>
#include <netdb.h>
#include <netinet/tcp.h>

#include "client.h"


//Splits string. Saves result to instance of class Out .

template<typename Out>
void split(const std::string& s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

//Specialization of above template for class std::vector<std::string>

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

//Gets information about server which address is IPv6
sockaddr_in6 getInfoIPv6(const char* host, uint16_t port, int socketType) {
    struct addrinfo addr_hints;
    struct addrinfo* addr_result;
    struct sockaddr_in6 my_address;

    memset(&addr_hints, 0, sizeof(struct addrinfo));

    addr_hints.ai_family = AF_INET6;
    addr_hints.ai_socktype = SOCK_DGRAM;
    addr_hints.ai_protocol = socketType;
    addr_hints.ai_flags = 0;
    addr_hints.ai_addrlen = 0;
    addr_hints.ai_addr = NULL;
    addr_hints.ai_canonname = NULL;
    addr_hints.ai_next = NULL;
    if (getaddrinfo(host, NULL, &addr_hints, &addr_result) != 0) {
        std::cerr << "Error with getting address info" << std::endl;
        exit(ERROR);
    }

    my_address.sin6_family = AF_INET6;
    my_address.sin6_addr =
            ((struct sockaddr_in6*) (addr_result->ai_addr))->sin6_addr;
    my_address.sin6_port = htons(port);

    freeaddrinfo(addr_result);

    return my_address;
}

//As above, but for IPv4
sockaddr_in getInfoIPv4(const char* host, uint16_t port, int socketType) {
    struct addrinfo addr_hints;
    struct addrinfo* addr_result;
    struct sockaddr_in my_address;

    memset(&addr_hints, 0, sizeof(struct addrinfo));

    addr_hints.ai_family = AF_INET;
    addr_hints.ai_socktype = SOCK_DGRAM;
    addr_hints.ai_protocol = socketType;
    addr_hints.ai_flags = 0;
    addr_hints.ai_addrlen = 0;
    addr_hints.ai_addr = NULL;
    addr_hints.ai_canonname = NULL;
    addr_hints.ai_next = NULL;

    if (getaddrinfo(host, NULL, &addr_hints, &addr_result) != 0) {
        std::cerr << "Error with getting address info" << std::endl;
        exit(ERROR);
    }

    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr =
            ((struct sockaddr_in*) (addr_result->ai_addr))->sin_addr.s_addr;
    my_address.sin_port = htons(port);

    freeaddrinfo(addr_result);

    return my_address;
}

// Creates instance of UDPMessaenger for easier communication
// with server
UDPMessenger getServerConnection(
        const std::string& serverHost,
        sockaddr_in6& serverInfoIP6,
        sockaddr_in& serverInfoIPv4
) {
    sockaddr* serverPointer = (sockaddr*) &serverInfoIPv4;
    socklen_t serverSize = (socklen_t) sizeof(serverInfoIPv4);
    static const uint16_t defaultServerPort = 12345;
    int serverSocket;

    auto serverElems = split(serverHost, ':');

    if (serverElems.size() == 1) {
        serverInfoIPv4 =
                getInfoIPv4(serverHost.c_str(), defaultServerPort, IPPROTO_UDP);
        serverSocket = socket(PF_INET, SOCK_DGRAM, 0);
    }
    else if (serverElems.size() == 2) {
        if (!isNum(serverElems[1].c_str())) {
            std::cerr << "Port should be a number" << std::endl;
            exit(ERROR);
        }

        auto port = atoi(serverElems[1].c_str());
        std::cout << serverElems[1] << std::endl;

        if (port > UINT16_MAX || port < 0) {
            std::cerr << "Invalid serverPort number" << std::endl;
            exit(ERROR);
        }

        serverInfoIPv4 = getInfoIPv4(
                serverElems[0].c_str(), (uint16_t) port, IPPROTO_UDP
        );
        serverSocket = socket(PF_INET, SOCK_DGRAM, 0);
    }
    else {
        serverInfoIP6 =
                getInfoIPv6(serverHost.c_str(), defaultServerPort, IPPROTO_UDP);
        serverSocket = socket(PF_INET6, SOCK_DGRAM, 0);
        serverPointer = (sockaddr*) &serverInfoIP6;
        serverSize = (socklen_t) sizeof serverInfoIP6;
    }

    UDPMessenger serverConnection(serverSocket, serverPointer, serverSize);
    return serverConnection;
}

// Gives socket to connect with GUI
int connectWithGui(
        const std::string& guiHost, uint16_t guiPort, bool isGuiIp6
) {
    struct addrinfo addr_hints;
    struct addrinfo* addr_result;
    int guiSocket;
    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_family = isGuiIp6 ? AF_INET6 : AF_INET;
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = IPPROTO_TCP;
    std::cout << guiHost << " " << guiPort << std::endl;
    auto err = getaddrinfo(
            guiHost.c_str(),
            numberToString(guiPort).c_str(),
            &addr_hints,
            &addr_result
    );

    if (err == EAI_SYSTEM) {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
        exit(ERROR);
    }
    else if (err != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
        exit(ERROR);
    }

    guiSocket = socket(
            addr_result->ai_family,
            addr_result->ai_socktype,
            addr_result->ai_protocol
    );

    if (guiSocket < 0) {
        std::cerr << "Connection error" << std::endl;
        exit(ERROR);
    }
    if (connect(guiSocket, addr_result->ai_addr, addr_result->ai_addrlen) < 0) {
        std::cerr << "Connection with GUI failed" << std::endl;
        exit(ERROR);
    }

    freeaddrinfo(addr_result);
    return guiSocket;
}

// Creates instance of TCPMessenger for easier communication
// with GUI
TCPMessenger getGUIConnection(int argc, char** argv) {
    uint16_t guiPort = 12346;
    std::string guiHost = "localhost";
    int guiSocket;
    bool isGuiIp6 = false;

    if (argc == 4) {
        std::string guiTerminalHost = std::string(argv[3]);
        auto guiElems = split(guiTerminalHost, ':');

        if (guiElems.size() != 2) {
            guiHost = guiTerminalHost;
        }
        else {
            guiHost = guiElems[0];
            if (!isNum(guiElems[1].c_str())) {
                std::cerr << "Port should be a number" << std::endl;
                exit(ERROR);
            }

            std::cout << guiElems[1] << std::endl;

            auto port = atoi(guiElems[1].c_str());

            if (port > UINT16_MAX || port < 0) {
                std::cerr << "Invalid serverPort number" << std::endl;
                exit(ERROR);
            }
            guiPort = (uint16_t) port;
        }

        if (guiElems.size() > 2) {
            isGuiIp6 = true;
        }
    }

    guiSocket = connectWithGui(guiHost, guiPort, isGuiIp6);
    int flag = 1;
    int result = setsockopt(
            guiSocket, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
    if (result < 0) {
        std::cerr << "Error while turning off nagle algorithm" << std::endl;
        exit(ERROR);
    }

    return TCPMessenger(guiSocket);
}

int main(int argc, char** argv) {
    if (argc != 3 && argc != 4) {
        std::cerr
                << "Usage: ./"
                << argv[0]
                << " player_name game_server_host[:serverPort] [ui_server_host[:serverPort]]";
        exit(ERROR);
    }

    //checking player name
    std::string playerName = std::string(argv[1]);

    if (!isValidPlayerName(playerName)) {
        std::cerr << "Invalid player name" << std::endl;
    }

    //We need pointers to make some things faster.
    //However, raw pointers are not so cool
    std::shared_ptr<std::string> playerNamePtr = std::shared_ptr<std::string>(
            new std::string(std::move(playerName)));

    //getting info about server
    std::string serverHost = std::string(argv[2]);
    sockaddr_in6 serverInfoIPv6;
    sockaddr_in serverInfoIPv4;

    UDPMessenger serverConnection =
            getServerConnection(serverHost, serverInfoIPv6, serverInfoIPv4);

    //connecting with gui
    TCPMessenger guiConnection = getGUIConnection(argc, argv);

    //creating client
    Client mainClient
            (std::move(guiConnection), serverConnection, playerNamePtr);

    try {
        //executing main loop
        mainClient.startCommunication();
    }
    catch (std::exception& e) {
        std::cerr
                << "Exception happened. Client will die: "
                << e.what()
                << std::endl;
        exit(ERROR);
    }
    catch (...) {
        std::cerr
                << "Something really unexpected happened. Client will die"
                << std::endl;
        exit(ERROR);
    }

    return 0;
}