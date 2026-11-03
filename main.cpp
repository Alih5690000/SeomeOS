#include <iostream>
#include <vector>
#include <windows.h>
#include <functional>
#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <sstream>

std::future_status getFutureStatus(std::future<int>& fut) {
    using namespace std::chrono_literals;
    auto status = fut.wait_for(0ms);
    return status;
}

//ACCESS ONLY UNDER MUTEX (NOT THREAD SAFE)
namespace Globals{
    std::map<std::string, std::function<int(std::vector<std::string>)>> Commands;
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
    Process(int (*f)(), int p=1):func(f), priority(p){}
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

int SayHello(std::vector<std::string> args){
    std::cout<<"Hello from SayHello!"<<std::endl;
    return 0;
}

int Leave(std::vector<std::string> args){
    std::cout<<"Exiting..."<<std::endl;
    std::exit(0);
}

int _secs=0;
bool _finished=false;

int timerFunction(){
    std::this_thread::sleep_for(std::chrono::seconds(_secs));
    std::cout << "Timer finished after " << _secs << " seconds." << std::endl;
    _finished = true;
    return 0;
}

int SetTimer(std::vector<std::string> args){
    if (args.size() < 1) {
        std::cout << "Usage: settimer <seconds>" << std::endl;
        return -1;
    }
    int seconds = std::stoi(args[0]);
    _secs = seconds;
    Process timerProcess(timerFunction);
    return 0;
}

int isTimerFinished(std::vector<std::string> args){
    if (_finished) {
        std::cout << "Timer has finished." << std::endl;
    } else {
        std::cout << "Timer is still running." << std::endl;
    }
    return 0;
}

int main(){
    {
        std::lock_guard<std::mutex> lock(Globals::CommandsMutex);
        Globals::Commands["sayhello"] = SayHello;
        Globals::Commands["exit"] = Leave;
        Globals::Commands["settimer"] = static_cast<int(*)(std::vector<std::string>)>(SetTimer);
        Globals::Commands["istimerfinished"] = isTimerFinished;
    }
    Core core;
    while (true){
        std::string name;
        std::string args;
        std::cout<<"$user>";
        std::cin>>name;
        std::getline(std::cin, args);
        std::lock_guard<std::mutex> lock(Globals::CommandsMutex);
        if (Globals::Commands.find(name) != Globals::Commands.end()) {
            std::vector<std::string> argList;
            std::stringstream ss(args);
            std::string arg;
            while (ss >> arg) {
                argList.push_back(arg);
            }
            Globals::Commands[name](argList);
        }
        else{
            std::cout<<"[Terminal] Command not found:"<<name<<std::endl;
        }
    }
    return 0;
}