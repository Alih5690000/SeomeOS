#include <iostream>
#include "lang.h"

int main(){
    Machine m;
    std::vector<std::uint8_t> coms={
        (uint8_t)COMMANDS::POINT, SPLIT_U32(2), 'p', 'd',
        (uint8_t)COMMANDS::POINT, SPLIT_U32(2), 'p', 't',
        (uint8_t)COMMANDS::BUFF, SPLIT_U32(2), 'O', 'K', (uint8_t)COMMANDS::PRINT,
        (uint8_t)COMMANDS::JMP, SPLIT_U32(2), 'p', 't'
    };
    /*const uint8_t s[]="lmao";
    m.buff.resize(sizeof(s));
    memcpy(m.buff.data(), s, sizeof(s));*/
    m.cache(coms);
    m.exec(coms);
    return 0;
}