#include <iostream>
#include <vector>
#include <windows.h>
#include <functional>
#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>

//ACCESS ONLY UNDER MUTEX (NOT THREAD SAFE)
namespace Globals{
    std::map<std::string, std::function<void(std::string)>> Commands;
    std::mutex CommandsMutex;
}

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
    std::mutex procMutex;
    std::vector<Process> processes;
};

int main(){
    Core core;
    std::thread(std::launch::async, [&](){
        while (true){
            {
                std::lock_guard<std::mutex> lock(core.procMutex);
                for (auto &process:core.processes){
                    process.update();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }).detach();
    while (true){
        std::string name;
        std::string args;
        std::cin>>name;
        std::getline(std::cin, args);
        std::lock_guard<std::mutex> lock(Globals::CommandsMutex);
        if (Globals::Commands.find(name) != Globals::Commands.end()) {
            Globals::Commands[name](args);
        }
        else{
            std::cout<<"Command not found:"<<name<<std::endl;
        }
    }
    return 0;
}