#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include "udpmessenger.h"

UDPMessenger::UDPMessenger(
        int socket, sockaddr* partnerAddress, socklen_t partnerAddressLength
)
        :
        communicationSocket(socket),
        partnerInfo(partnerAddress),
        partnerSize(partnerAddressLength) {
}

void UDPMessenger::sendMessage(
        const char* messageToSend, const size_t messageLength
) const {

    if (messageToSend == nullptr) {
        throw NullPointerException("Message to send cannot be NULL");
    }

    // std::cout << "sending " << messageLength << " bytes" << std::endl;
    ssize_t sentMessageLength = sendto(
            communicationSocket,
            messageToSend,
            messageLength,
            flags,
            partnerInfo,
            partnerSize
    );

    if (sentMessageLength != (ssize_t) messageLength) {
        throw SendException("Sent length differs from requested length");
    }
}

size_t UDPMessenger::getMessageAndSenderData(
        char* bufferForMessage,
        const size_t maximumMessageLength,
        sockaddr* addressToWrite,
        socklen_t* addressSize
) const {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Message to send cannot be NULL");
    }

    ssize_t receivedMessageLength = recvfrom(
            communicationSocket,
            bufferForMessage,
            maximumMessageLength,
            flags,
            addressToWrite,
            addressSize
    );
    if (*addressSize < 0) {
        throw ReceiveException("Error while getting the message");
    }
    if (receivedMessageLength == 0) {
        throw ReceiveException("Peer has performed an orderly shutdown");
    }
    if (receivedMessageLength < 0) {
        throw ReceiveException(
                "An error has occured. Received error code: " +
                numberToString(receivedMessageLength));
    }

    return static_cast<size_t>(receivedMessageLength);
}

size_t UDPMessenger::getMessage(
        char* bufferForMessage, const size_t maximumMessageLength
) const {
    if (bufferForMessage == nullptr) {
        throw NullPointerException("Message to send cannot be NULL");
    }

    sockaddr receivedAddress;
    socklen_t receivedAddressSize = partnerSize;

    ssize_t receivedMessageLength = recvfrom(
            communicationSocket,
            bufferForMessage,
            maximumMessageLength,
            flags,
            &receivedAddress,
            &receivedAddressSize
    );
    if (receivedAddressSize < 0) {
        throw ReceiveException("Error while getting the message");
    }
    if (receivedMessageLength == 0) {
        throw ReceiveException("Peer has performed an orderly shutdown");
    }
    if (receivedMessageLength < 0) {
        throw ReceiveException(
                "An error has occured. Received error code: " +
                numberToString(receivedMessageLength));
    }

    if (receivedAddressSize != partnerSize) {
        throw InvalidPartnerException(
                "Message from wrong partner. Invalid size: " + numberToString(
                        receivedAddressSize
                ) + " != " + numberToString(partnerSize));
    }

    if (partnerInfo->sa_family != receivedAddress.sa_family) {
        throw InvalidPartnerException(
                "Got message from wrong family: " +
                numberToString(partnerInfo->sa_family) +
                "!=" +
                numberToString(
                        receivedAddress.sa_family
                ));
    }
    else if (partnerInfo->sa_family == AF_INET) {
        sockaddr_in* receivedIP4 = (sockaddr_in*) &receivedAddress;
        sockaddr_in* partnerIP4 = (sockaddr_in*) partnerInfo;

        if (receivedIP4->sin_port != partnerIP4->sin_port ||
            receivedIP4->sin_addr.s_addr != partnerIP4->sin_addr.s_addr) {

            throw InvalidPartnerException("Message from invalid partner");
        }
    }
    else if (partnerInfo->sa_family == AF_INET6) {
        sockaddr_in6* receivedIP6 = (sockaddr_in6*) &receivedAddress;
        sockaddr_in6* partnerIP6 = (sockaddr_in6*) partnerInfo;

        if (receivedIP6->sin6_port != partnerIP6->sin6_port || (
                memcmp(
                        &(receivedIP6->sin6_addr),
                        &(partnerIP6->sin6_addr),
                        sizeof receivedIP6->sin6_addr
                ) != 0
        )) {
            throw InvalidPartnerException("Message from invalid partner");
        }
    }
    else {
        throw UnexpectedSituationException(
                "Somehow we have different family than IPv4 or IPv6"
        );
    }

    return static_cast<size_t>(receivedMessageLength);
}

int UDPMessenger::getSocket() {
    return communicationSocket;
}