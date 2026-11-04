#include <string>
#include <future>

typedef long long PROCESS;

PROCESS GetProc(std::string name);
std::future<int> runAsync(std::string name);
int run(std::string name);
void Init(void* p);
#define INIT extern"C" __declspec(dllexport) void init(void* a){Init(a);}