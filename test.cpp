#include "api.h"
#include <iostream>

__declspec(dllexport) int main(){
    std::cout<<"Hello from module"<<std::endl;
}

INIT