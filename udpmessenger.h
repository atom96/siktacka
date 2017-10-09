#ifndef SIK_DUZE_UDPMESSENGER_H
#define SIK_DUZE_UDPMESSENGER_H

#include "helper.h"

//represents udp connection
class UDPMessenger {
private:
    int communicationSocket;
    sockaddr* partnerInfo;
    socklen_t partnerSize;
    static const int flags = 0;

public:
    UDPMessenger(
            int socket, sockaddr* partnerAddress, socklen_t partnerAddressLength
    );

    void sendMessage(
            const char* messageToSend, const size_t messageLength
    ) const;

    size_t getMessage(
            char* bufferForMessage, const size_t maximumMessageLength
    ) const;

    size_t getMessageAndSenderData(
            char* bufferForMessage,
            const size_t maximumMessageLength,
            sockaddr* addressToWrite,
            socklen_t* addressSize
    ) const;

    int getSocket();
};

#endif //SIK_DUZE_UDPMESSENGER_H
