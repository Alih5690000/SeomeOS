#include <iostream>
#include <vector>
#include <windows.h>
#include <functional>
#include <map>
#include <thread>
#include <future>
#include <mutex>

std::map<std::string, std::function<void(std::string)>> Commands;

struct Process{
    void (*func)();
    int priotity=1;
    public:
    Process(std::string path){
        HMODULE sym=LoadLibraryA(path.c_str());
        func=(void(*)())GetProcAddress(sym, "main");
    }
    void update(){
        func();
    }
};

struct Core{
    std::vector<Process> processes;
};

int main(){
    Core core;
    std::thread(std::launch::async, [&](){
        while (true){
            for (auto &process:core.processes){
                process.update();
            }
        }
    }).detach();
    while (true){
        std::string name;
        std::string args;
        std::cin>>name;
        std::getline(std::cin, args);
        if (Commands.find(name) != Commands.end()) {
            Commands[name](args);
        }
        else{
            std::cout<<"Command not found:"<<name<<std::endl;
        }
    }
    return 0;
}