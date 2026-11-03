#include <vector>
#include <functional>
#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <sstream>
#include <windows.h>

std::future_status getFutureStatus(std::future<int>& fut) {
    using namespace std::chrono_literals;
    auto status = fut.wait_for(0ms);
    return status;
}

template <typename Core>
struct Process{
    int (*func)(Core&)=nullptr;
    int priority=1;
    unsigned long long  id;
    Core* c;
    std::string name;
    int launch(){
        return func(*c);
    }
    std::future<int> launchAsync(){
        return std::async(std::launch::async, func, std::ref(*c));
    }
    public:
    Process(std::string path, Core& core, std::string n):c(&core), name(n){
        HMODULE sym=LoadLibraryA(path.c_str());
        func=(int(*)(Core&))GetProcAddress(sym, "main");
    }
    Process(int (*f)(Core&), int p, Core& core, std::string n):func(f), priority(p), c(&core), name(n){}
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

__declspec(dllexport) Core core;