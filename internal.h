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
        return func();
    }
    int launch() const{
        return func();
    }
    std::future<int> launchAsync(){
        return std::async(std::launch::async, func);
    }
    public:
    Process(std::string path, Core& core, std::string n)
    : c(&core), name(n)
    {
        HMODULE sym = LoadLibraryA(path.c_str());

        if (!sym)
            throw std::runtime_error("Failed to load DLL");

        func = (int(*)())GetProcAddress(sym, "main");
        auto f = (void(*)(void*))GetProcAddress(sym, "init");

        if (!func || !f)
            throw std::runtime_error("DLL missing main or init export");

        f(&core);
    }
    Process(int (*f)(), Core& core, std::string n):func(f), c(&core), name(n){}
};

struct Core{
    unsigned long long nextProcessId=0;
    std::mutex procMutex;
    std::vector<Process<Core>> processes;
    Core()=default;
    std::future<int> addProcessAsync(const Process<Core>& proc){
        std::lock_guard<std::mutex> lock(procMutex);
        processes.emplace_back(proc);
        auto& p=processes.back();
        p.id = nextProcessId++;
        auto res=p.launchAsync();
        return res;
    }
    int addProcess(const Process<Core>& proc){
        return proc.launch();
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

Core* core;