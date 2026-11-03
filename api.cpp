#include "internal.h"
#include <string>
#include "api.h"

PROCESS GetProc(std::string name){
    auto& p=core.processes;
    for (int i=0;i<p.size();i++){
        if (p[i].name==name) return i;
    }
    throw std::runtime_error("No process named "+name);
}