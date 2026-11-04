#include "api.h"
#include <iostream>

extern"C" __declspec(dllexport) int main(){
    std::cout<<"Hello from module"<<std::endl;
}

INIT