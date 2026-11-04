#include <string>
#include <future>

typedef long long PROCESS;

PROCESS GetProc(std::string name);
std::future<int> run(std::string name);