#include <vector>
#include <functional>
#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <sstream>
#include <windows.h>
#include "main.hpp"

std::future_status getFutureStatus(std::future<int>& fut) {
    using namespace std::chrono_literals;
    auto status = fut.wait_for(0ms);
    return status;
}

template <typename Core>
struct Process{
    int (*func)()=nullptr;
    int priority=1;
    unsigned long long  id;
    Core* c;
    std::string name;
    int launch(){
        return func(*c);
    }
    std::future<int> launchAsync(){
        return std::async(std::launch::async, func);
    }
    public:
    Process(std::string path, Core& core, std::string n):c(&core), name(n){
        HMODULE sym=LoadLibraryA(path.c_str());
        func=(int(*)())GetProcAddress(sym, "main");
    }
    Process(int (*f)(), int p, Core& core, std::string n):func(f), priority(p), c(&core), name(n){}
};

struct Core{
    unsigned long long nextProcessId=0;
    std::mutex procMutex;
    std::vector<Process<Core>> processes;
    Core()=default;
    template<typename T>
    std::future<int> addProcess(const T& proc){
        std::lock_guard<std::mutex> lock(procMutex);
        processes.emplace_back(proc);
        auto& p=processes.back();
        p.id = nextProcessId++;
        auto res=p.launchAsync();
        return res;
    }
    std::vector<Process<Core>> getProcesses(){
        std::lock_guard<std::mutex> lock(procMutex);
        return processes;
    }
};

struct FileSystem{
    static DataBase db;
    FileSystem()=default;
    void WriteDataTo(const std::string& key, const std::vector<uint8_t>& data){
        db.attrs[key]=data;
    }
    void ReadDataFrom(const std::string& key, std::vector<uint8_t>& data){
        if (db.attrs.find(key) != db.attrs.end()){
            data=db.attrs[key];
        }
        else{
            throw std::runtime_error("Key not found in filesystem: "+key);
        }
    }
    void DumpToFile(const std::string& path){
        db.WriteTo(path);
    }
};

DataBase FileSystem::db("filesystem.db");

extern Core core;