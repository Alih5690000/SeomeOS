#include <iostream>
#include <vector>
#include <windows.h>
#include <functional>
#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>

std::future_status getFutureStatus(std::future<int>& fut) {
    using namespace std::chrono_literals;
    auto status = fut.wait_for(0ms);
    return status;
}

//ACCESS ONLY UNDER MUTEX (NOT THREAD SAFE)
namespace Globals{
    std::map<std::string, std::function<void(std::string)>> Commands;
    std::mutex CommandsMutex;
}

struct Process{
    int (*func)();
    int priority=1;
    public:
    Process(std::string path){
        HMODULE sym=LoadLibraryA(path.c_str());
        func=(int(*)())GetProcAddress(sym, "main");
    }
    int launch(){
        return func();
    }
    std::future<int> launchAsync(){
        return std::async(std::launch::async, func);
    }
};

struct Core{
    std::mutex procMutex;
    std::vector<Process> processes;
};

int main(){
    Core core;
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