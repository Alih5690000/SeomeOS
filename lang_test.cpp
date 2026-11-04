#include <iostream>
#include "lang.h"

int main(){
    Machine m;
    std::vector<std::uint8_t> coms={
        BUFF, SPLIT_U32(2), 'O', 'K', PRINT
    };
    /*const uint8_t s[]="lmao";
    m.buff.resize(sizeof(s));
    memcpy(m.buff.data(), s, sizeof(s));*/
    m.exec(coms);
    return 0;
}