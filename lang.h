#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <map>
#ifdef DEBUG
#define LOG(x) std::cout<<x<<std::endl;
#else 
#define LOG(x)
#endif

#define SPLIT_U32(x)                   \
    (uint8_t)((x) & 0xFF),             \
        (uint8_t)(((x) >> 8) & 0xFF),  \
        (uint8_t)(((x) >> 16) & 0xFF), \
        (uint8_t)(((x) >> 24) & 0xFF)

enum class COMMANDS
{
    PRINT,
    BUFF,
    POINT,
    JMP,
    ADD,
    SUB,
    REG,
    PUSH,
    POP,
    REG_TO_BUFF,
    BUFF_TO_REG
};

class Machine
{
public:
    std::vector<std::uint8_t> buff;
    std::vector<std::vector<std::uint8_t>> stack;
    std::vector<std::vector<std::uint8_t>> reg;
    std::map<std::string, int> points;
    Machine()
    {
        reg.resize(256);
    }
    void cache(std::vector<std::uint8_t> arr)
    {
        points.clear();

        for (std::size_t i = 0; i < arr.size(); i++)
        {

            if (arr[i] == (uint8_t)COMMANDS::BUFF ||
                arr[i] == (uint8_t)COMMANDS::PUSH ||
                arr[i] == (uint8_t)COMMANDS::POP ||
                arr[i] == (uint8_t)COMMANDS::REG_TO_BUFF ||
                arr[i] == (uint8_t)COMMANDS::BUFF_TO_REG)
            {

                if (arr.size() - i - 1 < 4)
                    throw std::runtime_error("Invalid instruction");

                uint32_t num;
                i++;
                memcpy(&num, arr.data() + i, 4);
                i += 4;

                // BUFF contains raw data.
                if (arr[i - 5] == (uint8_t)COMMANDS::BUFF)
                {
                    if (num > arr.size() - i)
                        throw std::runtime_error("Invalid BUFF data");

                    if (num > 0)
                        i += num - 1;
                }
            }

            else if (arr[i] == (uint8_t)COMMANDS::REG)
            {
                if (arr.size() - i - 1 < 8)
                    throw std::runtime_error("Invalid REG instruction");

                i++;

                uint32_t num, len;

                memcpy(&num, arr.data() + i, 4);
                i += 4;

                memcpy(&len, arr.data() + i, 4);
                i += 4;

                if (num >= reg.size())
                    throw std::runtime_error("Invalid register index");

                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid REG data");

                if (len > 0)
                    i += len - 1;
            }

            else if (arr[i] == (uint8_t)COMMANDS::POINT)
            {
                if (arr.size() - i - 1 < 4)
                    throw std::runtime_error("Invalid POINT instruction");

                i++;

                uint32_t len;
                memcpy(&len, arr.data() + i, 4);
                i += 4;

                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid POINT name");

                std::string res(len, '\0');
                memcpy(res.data(), arr.data() + i, len);

                if (len > 0)
                    i += len - 1;

                points[res] = i + 1;
            }

            else if (arr[i] == (uint8_t)COMMANDS::JMP)
            {
                if (arr.size() - i - 1 < 4)
                    throw std::runtime_error("Invalid JMP instruction");

                i++;

                uint32_t len;
                memcpy(&len, arr.data() + i, 4);
                i += 4;

                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid JMP destination");

                if (len > 0)
                    i += len - 1;
            }
        }
    }
    void exec(std::vector<std::uint8_t> arr)
    {
        for (int i = 0; i < arr.size(); i++)
        {
            if (arr[i] == (uint8_t)COMMANDS::PRINT)
            {
                std::string str;
                for (auto i : buff)
                {
                    str.push_back((char)i);
                }
                std::cout << str;
            }
            else if (arr[i] == (uint8_t)COMMANDS::BUFF)
            {
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid BUFF data");
                buff.resize(len);
                memcpy(buff.data(), arr.data() + i, len);
                i += len - 1;
            }
            else if (arr[i] == (uint8_t)COMMANDS::PUSH)
            {
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid PUSH instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                stack.push_back(reg[len]);
            }
            else if (arr[i] == (uint8_t)COMMANDS::REG_TO_BUFF)
            {
                LOG("[REG_TO_BUFF]");
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid REG_TO_BUFF instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t)-1;
                LOG("[REG_TO_BUFF]:DATA AT REG "+std::to_string(len));
                for (auto i:reg[len]){
                    LOG((char)i<<' ');
                }
                std::cout<<std::endl;
                buff = reg[len];
                LOG("[REG_TO_BUFF]:DATA AT BUFF");
                for (auto i:buff){
                    LOG((char)i<<' ');
                }
                std::cout<<std::endl;
            }
            else if (arr[i] == (uint8_t)COMMANDS::BUFF_TO_REG)
            {
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid REG_TO_BUFF instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                reg[len] = buff;
            }
            else if (arr[i] == (uint8_t)COMMANDS::POP)
            {
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid PUSH instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                auto back = stack.back();
                stack.pop_back();
                reg[len] = back;
            }
            else if (arr[i] == (uint8_t)COMMANDS::REG)
            {
                i++;
                uint32_t num;
                memcpy(&num, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                uint32_t len;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                std::vector<uint8_t> data;
                data.resize(len);
                memcpy(data.data(), arr.data() + i, len);
                i += len - 1;
                reg[num] = data;
            }
            else if (arr[i] == (uint8_t)COMMANDS::POINT)
            {
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid POINT name");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data() + i, len);
                i += len - 1;
                points[res] = i + 1;
            }
            else if (arr[i] == (uint8_t)COMMANDS::JMP)
            {
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid JMP destination");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data() + i, len);
                i += len;
                if (points.find(res) != points.end())
                {
                    i = points[res] - 1;
                }
                else
                {
                    throw std::runtime_error("Didnt find JMP destination");
                }
            }
            else if (arr[i] == (uint8_t)COMMANDS::ADD)
            {
                uint32_t first = *((uint32_t *)reg[0].data());
                uint32_t second = *((uint32_t *)reg[1].data());
                uint32_t res = first + second;
                reg[0].clear();
                reg[0].resize(sizeof(uint32_t));
                memcpy(reg[0].data(), &res, sizeof(uint32_t));
            }
            else if (arr[i] == (uint8_t)COMMANDS::SUB)
            {
                uint32_t first = *((uint32_t *)reg[0].data());
                uint32_t second = *((uint32_t *)reg[1].data());
                uint32_t res = first - second;
                reg[0].clear();
                reg[0].resize(sizeof(uint32_t));
                memcpy(reg[0].data(), &res, sizeof(uint32_t));
            }
        }
    }
};