//
// Created by arek on 02.06.17.
//

#ifndef SIK_DUZE_TCPMESSENGER_H
#define SIK_DUZE_TCPMESSENGER_H

#include <vector>
#include <unistd.h>
#include "helper.h"

//represents tcp connection
class TCPMessenger {
private:
    int mySocket;
    std::vector<char> temporaryBuffer;
    static const char separator = '\n';

public:
    TCPMessenger(int socket);

    void sendMessage(const char* messageToSend, const size_t messageLength);

    size_t getMessage(char* bufferForMessage, const size_t maxMessageLength);

    int getSocket();

    ~TCPMessenger();
};

#endif //SIK_DUZE_TCPMESSENGER_H
