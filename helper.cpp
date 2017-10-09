#include "helper.h"

RandomNumberGenerator::RandomNumberGenerator() : previousValue(time(NULL)) {
}

RandomNumberGenerator::RandomNumberGenerator(uint64_t seed) : previousValue(
        seed
) {
}

uint32_t RandomNumberGenerator::random32BitInteger() {
    uint32_t nextValue = (previousValue * defaultMultiplier) % defaultDivider;
    previousValue = nextValue;
    return nextValue;
}

/**
 * Tests whether we have number of not
 */

bool isNum(const char* strToTest) {
    if (strToTest == NULL) {
        return false;
    }

    unsigned int i = 0;
    char c = strToTest[0];

    while (c != '\0') {
        if (!isdigit(c)) {
            return false;
        }
        if (i >= 9) {
            return false;
        }
        ++i;

        c = strToTest[i];
    }

    return true;
}

bool isValidPlayerName(const std::string& playerName) {
    static const int minimalCharValue = 33;
    static const int maximalCharValue = 126;

    if (playerName.length() > maxNameLength) {
        return false;
    }

    for (auto c : playerName) {
        if (!(minimalCharValue <= c && c <= maximalCharValue)) {
            return false;
        }
    }

    return true;
}

