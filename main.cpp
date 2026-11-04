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

int timerFunction(){
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
    Process<Core> testProcess([](){
        std::cout<<"Hello from Proctest!"<<std::endl;
        return 0;
    }, 1, core, "Proctest");
    core.addProcess(testProcess);
    return 0;
}

const std::string DIR_SUFFIX = "_DIR" + std::string('\x1F', 1);

int MakeDir(std::vector<std::string> args, Core& core) {
    if (args.size() < 1) {
        std::cout << "Usage: mkdir <directory_name>" << std::endl;
        return -1;
    }

    std::string path = args[0];
    std::stringstream ss(path);
    std::string segment;
    std::vector<std::string> segments;

    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }

    if (segments.empty()) {
        std::cout << "Invalid path" << std::endl;
        return -1;
    }

    Ellement dirEl(segments.back() + DIR_SUFFIX, {}, true);

    if (segments.size() == 1) {
        FileSystem fs;
        fs.WriteDataTo(dirEl.name, dirEl.data);
        fs.DumpToFile("filesystem.db");
        return 0;
    }

    Map<Ellement> current =
        FileSystem::db.ReadMapEx(segments[0]);

    for (int i = 1; i < segments.size() - 1; i++) {
        current = current.Get(segments[i]).GetMap();
    }

    current.Set(segments.back(), dirEl);

    for (int i = segments.size() - 2; i > 0; i--) {
        Map<Ellement> parent =
            FileSystem::db.ReadMapEx(segments[i - 1]);

        parent.Set(segments[i], Ellement{current, true});
        current = parent;
    }

    FileSystem::db.AddMapEx(segments[0], current);
    FileSystem().DumpToFile("filesystem.db");

    return 0;
}

int DeleteFileFs(std::vector<std::string> args, Core& core) {
    if (args.empty()) {
        std::cout << "Usage: deletefile <path>\n";
        return -1;
    }

    std::vector<std::string> segments;
    std::stringstream ss(args[0]);
    std::string part;

    while (std::getline(ss, part, '/')) {
        if (!part.empty())
            segments.push_back(part);
    }

    if (segments.empty())
        return -1;

    auto eraseFromMap = [](Map<Ellement>& map, const std::string& name) {
        auto& v = map.v;

        v.erase(
            std::remove_if(v.begin(), v.end(),
                [&](const Ellement& el) {
                    return el.name == name;
                }),
            v.end()
        );
    };

    // Root-level entry
    if (segments.size() == 1) {
        FileSystem::db.attrs.erase(segments[0]);

        FileSystem().DumpToFile("filesystem.db");
        return 0;
    }

    // Find the parent directory
    Map<Ellement> current =
        FileSystem::db.ReadMapEx(segments[0]);

    for (int i = 1; i < segments.size() - 1; i++) {
        current = current.Get(segments[i]).GetMap();
    }

    // Delete the target
    eraseFromMap(current, segments.back());

    // Propagate modified maps back to the root
    for (int i = static_cast<int>(segments.size()) - 2; i > 0; i--) {
        Map<Ellement> parent =
            FileSystem::db.ReadMapEx(segments[i - 1]);

        parent.Set(segments[i], Ellement{current, true});
        current = parent;
    }

    FileSystem::db.AddMapEx(segments[0], current);

    FileSystem().DumpToFile("filesystem.db");
    return 0;
}

int WriteFs(std::vector<std::string> args, Core& core) {
    if (args.size() < 2) {
        std::cout << "Usage: writefs <file_name> <content>" << std::endl;
        return -1;
    }

    std::string path = args[0];
    std::stringstream ss(path);
    std::string segment;
    std::vector<std::string> segments;

    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }

    if (segments.empty()) {
        std::cout << "Invalid path" << std::endl;
        return -1;
    }

    if (segments.size()==1){
        FileSystem::db.AddStr(segments[0], args[1]);
        return 0;
    }

    Map<Ellement> current =
        FileSystem::db.ReadMapEx(segments[0]);

    for (int i = 1; i < segments.size() - 1; i++) {
        current = current.Get(segments[i]).GetMap();
    }

    current.Set(segments.back(), Ellement{args[1]});

    FileSystem::db.AddMapEx(segments[0], current);

    FileSystem().DumpToFile("filesystem.db");

    return 0;
}

int SaveFile(std::vector<std::string> args, Core& core){
    if (args.size() < 1) {
        std::cout << "Usage: createfile <file_name>" << std::endl;
        return -1;
    }
    std::string path = args[0];
    std::stringstream ss(path);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }
    if (segments.size()==1){
        FileSystem::db.AddStr(segments[0], "");
        FileSystem().DumpToFile("filesystem.db");
        return 0;
    }
    Map<Ellement> current=FileSystem::db.
        ReadMapEx(segments[0]);
    for (int i=1;i<segments.size()-1;i++){
        current=current.Get(segments[i]).GetMap();
    }
    current.Set(segments.back(), Ellement{""});
    if (segments.size() > 2) {
        for (int i = segments.size()-2; i > 0; i--) {
            Map m = FileSystem::db.ReadMapEx(segments[i-1]);
            m.Set(segments[i], Ellement{current, true});
            current = m;
        }
    }
    FileSystem::db.AddMapEx(segments[0], current);
    FileSystem().DumpToFile("filesystem.db");
    return 0;
}

int OpenFile(std::vector<std::string> args, Core& core){
    if (args.size() < 1) {
        std::cout << "Usage: openfile <file_name>" << std::endl;
        return -1;
    }

    std::string path = args[0];
    std::stringstream ss(path);
    std::string segment;
    std::vector<std::string> segments;

    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }

    if (segments.empty()) {
        std::cout << "Invalid path" << std::endl;
        return -1;
    }

    if (segments.size()==1){
        std::string str;
        str.resize(FileSystem::db.attrs[segments[0]].size());
        memcpy(str.data(), FileSystem::db.attrs[segments[0]].data(),
            FileSystem::db.attrs[segments[0]].size());
        std::cout<<str<<std::endl;
        return 0;
    }

    Map<Ellement> current =
        FileSystem::db.ReadMapEx(segments[0]);

    for (int i = 1; i < segments.size() - 1; i++) {
        current = current.Get(segments[i]).GetMap();
    }

    Ellement file = current.Get(segments.back());

    std::cout << "OpenFile log" << '\n';
    std::cout << "Name: " << file.name << '\n';
    std::cout << "Data size: " << file.data.size() << '\n';
    std::cout << "Value: " << file.GetStr() << '\n';

    std::cout << file.GetStr() << std::endl;

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

std::future<int> runRes;

int run(std::vector<std::string> args, Core& core){
    if (args.size()<1){
        std::cout << "Usage: run <file_name>" << std::endl;
        return -1;
    }
    runRes=core.addProcess(Process<Core>(args[0], core, args[0]));
    return 0;
}

int main(){
    core=new Core;
    if (FileSystem::db.attrs.find("inited") == FileSystem::db.attrs.end()) {
        FileSystem f;
        f.WriteDataTo("inited", std::vector<uint8_t>{1});
        f.DumpToFile("filesystem.db");
    }
    {
        std::lock_guard<std::mutex> lock(Globals::CommandsMutex);
        Globals::Commands["sayhello"] = SayHello;
        Globals::Commands["exit"] = Leave;
        Globals::Commands["settimer"] = static_cast<int(*)(std::vector<std::string>, Core&)>(SetTimer);
        Globals::Commands["istimerfinished"] = static_cast<int(*)(std::vector<std::string>, Core&)>(isTimerFinished);
        Globals::Commands["createfile"] = static_cast<int(*)(std::vector<std::string>, Core&)>(SaveFile);
        Globals::Commands["openfile"] = static_cast<int(*)(std::vector<std::string>, Core&)>(OpenFile);
        Globals::Commands["proctest"] = static_cast<int(*)(std::vector<std::string>, Core&)>(Proctest);
        Globals::Commands["writefs"] = static_cast<int(*)(std::vector<std::string>, Core&)>(WriteFs);
        Globals::Commands["mkdir"] = static_cast<int(*)(std::vector<std::string>, Core&)>(MakeDir);
        Globals::Commands["deletefile"] = static_cast<int(*)(std::vector<std::string>, Core&)>(DeleteFileFs);
        Globals::Commands["run"] = static_cast<int(*)(std::vector<std::string>, Core&)>(run);
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
            Globals::Commands[name](argList, *core);
        }
        else{
            std::cout<<"[Terminal] Command not found:"<<name<<std::endl;
        }
    }
    return 0;
}