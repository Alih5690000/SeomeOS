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
#include "main.hpp"
#include "internal.h"

//ACCESS ONLY UNDER MUTEX (NOT THREAD SAFE)
namespace Globals{
    std::map<std::string, std::function<int(std::vector<std::string>, Core&)>> Commands;
    std::mutex CommandsMutex;
}

int SayHello(std::vector<std::string> args, Core& core){
    std::cout<<"Hello from SayHello!"<<std::endl;
    return 0;
}

int Leave(std::vector<std::string> args, Core& core){
    std::cout<<"Exiting..."<<std::endl;
    std::exit(0);
}

int _secs=0;
bool _finished=false;

int timerFunction(Core& core){
    std::this_thread::sleep_for(std::chrono::seconds(_secs));
    _finished = true;
    return 0;
}

std::future<int> timerFuture;

int SetTimer(std::vector<std::string> args, Core& core){
    if (args.size() < 1) {
        std::cout << "Usage: settimer <seconds>" << std::endl;
        return -1;
    }
    int seconds = std::stoi(args[0]);
    _secs = seconds;
    Process timerProcess(timerFunction, 1, core, "Timer");
    timerFuture = core.addProcess(timerProcess);
    return 0;
}

int Proctest(std::vector<std::string> args, Core& core){
    std::cout<<"Starting Proctest..."<<std::endl;
    Process<Core> testProcess([](Core& core){
        std::cout<<"Hello from Proctest!"<<std::endl;
        return 0;
    }, 1, core, "Proctest");
    core.addProcess(testProcess);
    return 0;
}

int isTimerFinished(std::vector<std::string> args, Core& core) {
    if (_finished) {
        std::cout << "Timer has finished." << std::endl;
    } else {
        std::cout << "Timer is still running." << std::endl;
    }
    return 0;
}

int main(){
    DataBase db("filesystem.db");
    if (db.attrs.find("main") == db.attrs.end()) {
        std::vector<Ellement> mainElements;
        db.AddMapEx("main", mainElements);
    }
    {
        std::lock_guard<std::mutex> lock(Globals::CommandsMutex);
        Globals::Commands["sayhello"] = SayHello;
        Globals::Commands["exit"] = Leave;
        Globals::Commands["settimer"] = static_cast<int(*)(std::vector<std::string>, Core&)>(SetTimer);
        Globals::Commands["istimerfinished"] = static_cast<int(*)(std::vector<std::string>, Core&)>(isTimerFinished);
        Globals::Commands["proctest"] = static_cast<int(*)(std::vector<std::string>, Core&)>(Proctest);
    }
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
            Globals::Commands[name](argList, core);
        }
        else{
            std::cout<<"[Terminal] Command not found:"<<name<<std::endl;
        }
    }
    return 0;
}