#include <iostream>
#define DEBUG
#include "lang.h"

Machine m;

void transaletToStr(Machine& ma){
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

        (uint8_t)COMMANDS::PRINT,

        (uint8_t)COMMANDS::REG,
        SPLIT_U32(0),
        SPLIT_U32(4),
        SPLIT_U32(1),

        (uint8_t)COMMANDS::REG,
        SPLIT_U32(1),
        SPLIT_U32(4),
        SPLIT_U32(3),

        (uint8_t)COMMANDS::LESS,
        
        (uint8_t)COMMANDS::CALL,
        SPLIT_U64((ptrdiff_t)&transaletToStr),

        (uint8_t)COMMANDS::REG_TO_BUFF,
        SPLIT_U32(0),

        (uint8_t)COMMANDS::JMP_IF,
        SPLIT_U32(2),
        SPLIT_U32(0),
        'p', 't',

        (uint8_t)COMMANDS::REG,
        SPLIT_U32(0),
        SPLIT_U32(2),
        'n', 'o',

        (uint8_t)COMMANDS::REG_TO_BUFF,
        SPLIT_U32(0),

        (uint8_t)COMMANDS::PRINT,

        (uint8_t)COMMANDS::POINT,
        SPLIT_U32(2),
        'p', 't'
    };
    std::vector<uint8_t> coms2={
        (uint8_t)COMMANDS::REG,
        SPLIT_U32(0),
        SPLIT_U32(4),
        SPLIT_U32(1),

        (uint8_t)COMMANDS::PUSH_REG,
        SPLIT_U32(0),

        (uint8_t)COMMANDS::REG,
        SPLIT_U32(0),
        SPLIT_U32(4),
        SPLIT_U32(2),

        (uint8_t)COMMANDS::PUSH_REG,
        SPLIT_U32(0),

        (uint8_t)COMMANDS::POP_EX,
        SPLIT_U32(0),
        SPLIT_U32(0),

        (uint8_t)COMMANDS::CALL,
        SPLIT_U64((ptrdiff_t)&transaletToStr),

        (uint8_t)COMMANDS::REG_TO_BUFF,
        SPLIT_U32(0),

        (uint8_t)COMMANDS::PRINT
    };
    /*const uint8_t s[]="lmao";
    m.buff.resize(sizeof(s));
    memcpy(m.buff.data(), s, sizeof(s));*/
    m.cache(coms2);
    m.exec(coms2);
    return 0;
}