#ifndef SIK_DUZE_ADDITIONALEXCEPTIONS_H
#define SIK_DUZE_ADDITIONALEXCEPTIONS_H

#include <string>
#include <exception>

/*===========================================================================*
 *                        Class: SendException                               *
 *===========================================================================*/

class SendException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    SendException(std::string message);
};

/*===========================================================================*
 *                        Class: ReceiveException                            *
 *===========================================================================*/

class ReceiveException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    ReceiveException(std::string message);
};

/*===========================================================================*
 *                        Class: SocketException                             *
 *===========================================================================*/

class SocketException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    SocketException(std::string message);
};

/*===========================================================================*
 *                        Class: NullPointerException                        *
 *===========================================================================*/

class NullPointerException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    NullPointerException(std::string message);
};

/*===========================================================================*
 *                        Class: InvalidArgumentException                    *
 *===========================================================================*/

class InvalidArgumentException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    InvalidArgumentException(std::string message);
};

/*===========================================================================*
 *                        Class: UnexpectedSituationException                *
 *===========================================================================*/


class UnexpectedSituationException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    UnexpectedSituationException(std::string message);
};

/*===========================================================================*
 *                        Class: ControlSumException                         *
 *===========================================================================*/


class ControlSumException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    ControlSumException(std::string message);
};

/*===========================================================================*
 *                        Class: EventTypeException                          *
 *===========================================================================*/


class EventTypeException : public std::exception {
private:
    std::string errorMessage;
public:
    virtual const char* what() const throw();

    EventTypeException(std::string message);
};

/*===========================================================================*
 *                        Class: MessageSizeException                        *
 *===========================================================================*/


class MessageSizeException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    MessageSizeException(std::string message);
};

/*===========================================================================*
 *                        Class: ConstructorException                        *
 *===========================================================================*/

class ConstructorException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    ConstructorException(std::string message);
};

/*===========================================================================*
 *                        Class: UnimplementedMethodException                *
 *===========================================================================*/
class UnimplementedMethodException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    UnimplementedMethodException(std::string message);
};

/*===========================================================================*
 *                        Class: InvalidPartnerException                     *
 *===========================================================================*/
class InvalidPartnerException : public std::exception {
private:
    std::string errorMessage;

public:
    virtual const char* what() const throw();

    InvalidPartnerException(std::string message);
};

#endif //SIK_DUZE_ADDITIONALEXCEPTIONS_H
