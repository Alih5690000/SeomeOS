#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <map>

#define SPLIT_U32(x) \
    (uint8_t)((x) & 0xFF), \
    (uint8_t)(((x) >> 8) & 0xFF), \
    (uint8_t)(((x) >> 16) & 0xFF), \
    (uint8_t)(((x) >> 24) & 0xFF)

enum class COMMANDS{
    PRINT,
    BUFF,
    POINT,
    JMP,
    ADD,
    SUB,
    REG,
    PUSH,
    POP
};

class Machine{
    public:
    std::vector<std::uint8_t> buff;
    std::vector<std::vector<std::uint8_t>> stack;
    std::vector<std::vector<std::uint8_t>> reg;
    std::map<std::string, int> points;
    Machine(){
        reg.resize(256);
    }
    void cache(std::vector<std::uint8_t> arr){
        for (int i=0;i<arr.size();i++){
            if (arr[i]==(uint8_t)COMMANDS::BUFF){
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid BUFF data");
                i+=len-1;
            }
            if (arr[i]==(uint8_t)COMMANDS::REG){
                i++;
                uint32_t num;
                memcpy(&num, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                uint32_t len;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                i=+len-1;
            }
            if (arr[i]==(uint8_t)COMMANDS::POINT){
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid POINT name");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data()+i, len);
                i+=len-1;
                points[res]=i+1;
            }
            if (arr[i]==(uint8_t)COMMANDS::JMP){
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid JMP destination");
                i+=len;
            }
        }
    }
    void exec(std::vector<std::uint8_t> arr){
        for (int i=0;i<arr.size();i++){
            if (arr[i]==(uint8_t)COMMANDS::PRINT){
                std::string str;
                for (auto i:buff){
                    str.push_back((char)i);
                }
                std::cout<<str;
            }
            if (arr[i]==(uint8_t)COMMANDS::BUFF){
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid BUFF data");
                buff.resize(len);
                memcpy(buff.data(), arr.data()+i, len);
                i+=len-1;
            }
            if (arr[i]==(uint8_t)COMMANDS::PUSH){
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid PUSH instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid PUSH data");
                i+=len-1;
                stack.push_back(reg[len]);
            }
            if (arr[i]==(uint8_t)COMMANDS::POP){
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid PUSH instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid PUSH data");
                i+=len-1;
                auto back=stack.back();
                stack.pop_back();
                reg[len]=back;
            }
            if (arr[i]==(uint8_t)COMMANDS::REG){
                i++;
                uint32_t num;
                memcpy(&num, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                uint32_t len;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                std::vector<uint8_t> data;
                data.resize(len);
                memcpy(data.data(), arr.data()+i, len);
                i+=len-1;
                reg[num]=data;
            }
            if (arr[i]==(uint8_t)COMMANDS::POINT){
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid POINT name");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data()+i, len);
                i+=len-1;
                points[res]=i+1;
            }
            if (arr[i]==(uint8_t)COMMANDS::JMP){
                uint32_t len;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid JMP destination");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data()+i, len);
                i+=len;
                if (points.find(res)!=points.end()){
                    i=points[res]-1;
                }
                else{
                    throw std::runtime_error("Didnt find JMP destination");
                }
            }
            if (arr[i]==(uint8_t)COMMANDS::ADD){
                uint32_t first=*((uint32_t*)reg[0].data());
                uint32_t second=*((uint32_t*)reg[1].data());
                uint32_t res=first+second;
                reg[0].clear();
                reg[0].resize(sizeof(uint32_t));
                memcpy(reg[0].data(), &res, sizeof(uint32_t));
            }
            if (arr[i]==(uint8_t)COMMANDS::SUB){
                uint32_t first=*((uint32_t*)reg[0].data());
                uint32_t second=*((uint32_t*)reg[1].data());
                uint32_t res=first-second;
                reg[0].clear();
                reg[0].resize(sizeof(uint32_t));
                memcpy(reg[0].data(), &res, sizeof(uint32_t));
            }
        }
    }
};