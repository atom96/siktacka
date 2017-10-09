#ifndef SIK_DUZE_HELPER_H
#define SIK_DUZE_HELPER_H

#include <cstdint>
#include <cstdlib>
#include <netinet/in.h>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include "additional_exceptions.h"

#define ERROR 1

static const int maxNameLength = 64;

using milliseconds = std::chrono::milliseconds;

//return current time in milliseconds
milliseconds inline currentTime() {
    return std::chrono::duration_cast<milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
}

//genrates random numbers
class RandomNumberGenerator {
private:
    uint64_t previousValue;
    static const uint64_t defaultMultiplier = 279470273;
    static const uint64_t defaultDivider = 4294967291;

public:
    RandomNumberGenerator();

    RandomNumberGenerator(uint64_t seed);

    //returns random integer generated with algorithm
    //described in task specification
    uint32_t random32BitInteger();
};

// converts any type f number to std::string
template<typename T> std::string numberToString(T number) {
    if (number == 0) {
        return "0";
    }
    std::stringstream ss;
    ss << number;
    return ss.str();
}

//check whether given string is valid number
bool isNum(const char* strToTest);

//check whether given player name is valid
bool isValidPlayerName(const std::string& playerName);

#endif //SIK_DUZE_HELPER_H
