#include <iostream>
#define DEBUG
#include "lang.h"

Machine m;

void transaletToStr(){
    uint32_t num=*(uint32_t*)m.reg[0].data();
    LOG("[transaltor]:num is "+std::to_string(num));
    std::string str=std::to_string((int)num);
    m.reg[0].resize(str.size());
    memcpy(m.reg[0].data(), str.data(), str.size());
}

int main()
{
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

        (uint8_t)COMMANDS::CALL,
        SPLIT_U64((ptrdiff_t)&transaletToStr),

        (uint8_t)COMMANDS::REG_TO_BUFF,
        SPLIT_U32(0),

        (uint8_t)COMMANDS::PRINT
    };
    /*const uint8_t s[]="lmao";
    m.buff.resize(sizeof(s));
    memcpy(m.buff.data(), s, sizeof(s));*/
    m.cache(coms);
    m.exec(coms);
    return 0;
}