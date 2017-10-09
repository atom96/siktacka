#include <zconf.h>
#include <iostream>
#include "tcpmessenger.h"

TCPMessenger::TCPMessenger(int socket) : mySocket(socket) {
}

void TCPMessenger::sendMessage(
        const char* messageToSend, const size_t messageLength
) {
    auto writingResult = write(mySocket, messageToSend, messageLength);

    if (static_cast<size_t>(writingResult) != messageLength) {
        throw SendException("Not enough bytes sent");
    }
}

size_t TCPMessenger::getMessage(
        char* bufferForMessage, const size_t maxMessageLength
) {
    size_t readSize = 0;

    for (size_t i = 0; i < temporaryBuffer.size(); ++i) {
        bufferForMessage[0] = temporaryBuffer[i];
        ++bufferForMessage;
        ++readSize;
    }

    ssize_t readBytes =
            read(mySocket, bufferForMessage, maxMessageLength - readSize);

    if (readBytes < 0) {
        throw ReceiveException("Got error while receiving message");
    }
    else if (readBytes == 0) {
        throw ReceiveException("Connection closed");
    }

    int pos = -1;

    for (int i = 0; i < readBytes; ++i) {
        if (bufferForMessage[i] == separator) {
            pos = i;
            break;
        }
    }

    ++pos;

    if (pos == 0 && readBytes + readSize == maxMessageLength) {
        temporaryBuffer.clear();
        return maxMessageLength;
    }
    else if (pos != 0) {
        temporaryBuffer.clear();
    }

    readSize += pos;

    for (int i = pos; i < readBytes; ++i) {
        temporaryBuffer.push_back(bufferForMessage[i]);
    }

    return readSize;
}

int TCPMessenger::getSocket() {
    return mySocket;
}

TCPMessenger::~TCPMessenger() {
    close(mySocket);
}