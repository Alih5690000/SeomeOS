#include "internal.h"
#include <string>
#include <future>
#include "api.h"

PROCESS GetProc(std::string name){
    auto& p=core->processes;
    for (int i=0;i<p.size();i++){
        if (p[i].name==name) return i;
    }
    throw std::runtime_error("No process named "+name);
}

std::future<int> runAsync(std::string name){
    return core->addProcessAsync(Process<Core>(name, *core, name));
}

int run(std::string name){
    return core->addProcess(Process<Core>(name, *core, name));
}

void Init(void* p){
    core=(Core*)p;
}
