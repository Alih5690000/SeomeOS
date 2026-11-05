#include <iostream>
#define DEBUG
#include "lang.h"

int main()
{
    Machine m;
    std::vector<std::uint8_t> coms = {
        (uint8_t)COMMANDS::REG,
        SPLIT_U32(0),
        SPLIT_U32(4),
        SPLIT_U32(99),

        (uint8_t)COMMANDS::REG,
        SPLIT_U32(1),
        SPLIT_U32(4),
        SPLIT_U32(1),

        (uint8_t)COMMANDS::ADD,

        (uint8_t)COMMANDS::REG_TO_BUFF,
        SPLIT_U32(1),

        (uint8_t)COMMANDS::PRINT
    };
    /*const uint8_t s[]="lmao";
    m.buff.resize(sizeof(s));
    memcpy(m.buff.data(), s, sizeof(s));*/
    m.cache(coms);
    m.exec(coms);
    return 0;
}