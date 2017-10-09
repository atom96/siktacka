#include "additional_exceptions.h"

/*===========================================================================*
 *                        Class: SendException                               *
 *===========================================================================*/
SendException::SendException(std::string message) : errorMessage(message) {
}

const char* SendException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: ReceiveException                            *
 *===========================================================================*/
ReceiveException::ReceiveException(std::string message) : errorMessage(
        message
) {
}

const char* ReceiveException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: SocketException                             *
 *===========================================================================*/
SocketException::SocketException(std::string message) : errorMessage(message) {
}

const char* SocketException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: NullPointerException                        *
 *===========================================================================*/
NullPointerException::NullPointerException(std::string message) : errorMessage(
        message
) {
}

const char* NullPointerException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: InvalidArgumentException                    *
 *===========================================================================*/
InvalidArgumentException::InvalidArgumentException(std::string message)
        :
        errorMessage(message) {
}

const char* InvalidArgumentException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: UnexpectedSituationException                *
 *===========================================================================*/

UnexpectedSituationException::UnexpectedSituationException(std::string message)
        :
        errorMessage(message) {
}

const char* UnexpectedSituationException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: ControlSumException                         *
 *===========================================================================*/


ControlSumException::ControlSumException(std::string message)
        :
        errorMessage(message) {
}

const char* ControlSumException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: EventTypeException                          *
 *===========================================================================*/

EventTypeException::EventTypeException(std::string message)
        :
        errorMessage(message) {
}

const char* EventTypeException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: MessageSizeException                        *
 *===========================================================================*/

MessageSizeException::MessageSizeException(std::string message)
        :
        errorMessage(message) {
}

const char* MessageSizeException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: ConstructorException                        *
 *===========================================================================*/

ConstructorException::ConstructorException(std::string message)
        :
        errorMessage(message) {
}

const char* ConstructorException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: UnimplementedMethodException                *
 *===========================================================================*/

UnimplementedMethodException::UnimplementedMethodException(std::string message)
        :
        errorMessage(message) {
}

const char* UnimplementedMethodException::what() const throw() {
    return errorMessage.c_str();
}

/*===========================================================================*
 *                        Class: InvalidPartnerException                     *
 *===========================================================================*/

InvalidPartnerException::InvalidPartnerException(std::string message)
        :
        errorMessage(message) {
}

const char* InvalidPartnerException::what() const throw() {
    return errorMessage.c_str();
}
