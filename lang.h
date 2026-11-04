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

enum COMMANDS{
    PRINT,
    BUFF,
    POINT
};

class Machine{
    public:
    std::vector<std::uint8_t> buff;
    std::vector<std::vector<std::uint8_t>> regs;
    std::map<std::string, int> points;
    void exec(std::vector<std::uint8_t> arr){
        for (int i=0;i<arr.size();i++){
            if (arr[i]==PRINT){
                std::string str;
                for (auto i:buff){
                    str.push_back((char)i);
                }
                std::cout<<str;
            }
            if (arr[i]==BUFF){
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
            if (arr[i]==POINT){
                uint32_t len;
                int pointPos=i;
                i++;
                memcpy(&len, arr.data()+i, sizeof(uint32_t));
                i+=sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid POINT name");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data()+i, len);
                i+=len-1;
                points[res]=pointPos;
            }
        }
    }
};