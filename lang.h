#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <map>
#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl;
#else
#define LOG(x)
#endif

#define SPLIT_U32(x)                   \
    (uint8_t)((x) & 0xFF),             \
        (uint8_t)(((x) >> 8) & 0xFF),  \
        (uint8_t)(((x) >> 16) & 0xFF), \
        (uint8_t)(((x) >> 24) & 0xFF)

#define SPLIT_U64(x)                   \
    (uint8_t)((x) & 0xFF),             \
        (uint8_t)(((x) >> 8) & 0xFF),  \
        (uint8_t)(((x) >> 16) & 0xFF), \
        (uint8_t)(((x) >> 24) & 0xFF), \
        (uint8_t)(((x) >> 32) & 0xFF), \
        (uint8_t)(((x) >> 40) & 0xFF), \
        (uint8_t)(((x) >> 48) & 0xFF), \
        (uint8_t)(((x) >> 56) & 0xFF)

enum class COMMANDS
{
    PRINT,
    BUFF,
    POINT,
    JMP,
    ADD,
    SUB,
    GREATER,
    LESS,
    EQUAL,
    REG,
    PUSH,
    POP,
    RUN,
    REG_TO_BUFF,
    BUFF_TO_REG,
    APPEND_REG,
    CALL,
    JMP_IF,
    JMP_IF_NOT,
    RET_TO_REG,
    REG_TO_REG
};

class Machine
{
public:
    std::vector<std::uint8_t> buff;
    std::vector<std::vector<std::uint8_t>> stack;
    std::vector<std::vector<std::uint8_t>> reg;
    std::map<std::string, int> points;
    std::vector<uint8_t> ret;
    Machine()
    {
        reg.resize(256);
    }

    void cache(const std::vector<uint8_t> &arr)
    {
        points.clear();

        std::size_t i = 0;

        auto readU32 = [&]() -> uint32_t
        {
            if (arr.size() - i < sizeof(uint32_t))
                throw std::runtime_error("Invalid instruction");

            uint32_t value;
            memcpy(&value, arr.data() + i, sizeof(value));
            i += sizeof(value);

            return value;
        };

        auto skip = [&](std::size_t len, const char *error)
        {
            if (len > arr.size() - i)
                throw std::runtime_error(error);

            i += len;
        };

        while (i < arr.size())
        {
            COMMANDS cmd = (COMMANDS)arr[i++];

            switch (cmd)
            {
            // No operands
            case COMMANDS::PRINT:
            case COMMANDS::ADD:
            case COMMANDS::SUB:
            case COMMANDS::GREATER:
            case COMMANDS::LESS:
            case COMMANDS::EQUAL:
                break;

            // uint32 length + data
            case COMMANDS::BUFF:
            {
                uint32_t len = readU32();
                skip(len, "Invalid BUFF data");
                break;
            }

            // uint32 register + uint32 length + data
            case COMMANDS::REG:
            {
                uint32_t regIndex = readU32();
                uint32_t len = readU32();

                if (regIndex >= reg.size())
                    throw std::runtime_error("Invalid register index");

                skip(len, "Invalid REG data");
                break;
            }

            // One register index
            case COMMANDS::PUSH:
            case COMMANDS::POP:
            case COMMANDS::REG_TO_BUFF:
            case COMMANDS::BUFF_TO_REG:
            case COMMANDS::RET_TO_REG:
            {
                uint32_t regIndex = readU32();

                if (regIndex >= reg.size())
                    throw std::runtime_error("Invalid register index");

                break;
            }

            // Two register indices
            case COMMANDS::APPEND_REG:
            case COMMANDS::REG_TO_REG:
            {
                uint32_t dest = readU32();
                uint32_t src = readU32();

                if (dest >= reg.size() || src >= reg.size())
                    throw std::runtime_error("Invalid register index");

                break;
            }

            // Native function address
            case COMMANDS::CALL:
            {
                skip(sizeof(ptrdiff_t), "Invalid CALL instruction");
                break;
            }

            // Memory address + data length
            case COMMANDS::RUN:
            {
                skip(sizeof(ptrdiff_t), "Invalid RUN address");

                uint32_t len = readU32();

                // RUN references external memory, so there is no
                // bytecode payload to skip here.
                (void)len;

                break;
            }

            // Label declaration
            case COMMANDS::POINT:
            {
                uint32_t len = readU32();

                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid POINT name");

                std::string name(
                    reinterpret_cast<const char *>(arr.data() + i),
                    len);

                i += len;

                if (points.find(name) != points.end())
                    throw std::runtime_error("Duplicate POINT: " + name);

                points[name] = static_cast<int>(i);
                break;
            }

            // Unconditional jump
            case COMMANDS::JMP:
            {
                uint32_t len = readU32();
                skip(len, "Invalid JMP destination");
                break;
            }

            // Conditional jumps
            case COMMANDS::JMP_IF:
            case COMMANDS::JMP_IF_NOT:
            {
                uint32_t len = readU32();
                uint32_t regIndex = readU32();

                if (regIndex >= reg.size())
                    throw std::runtime_error("Invalid register index");

                skip(len, "Invalid conditional JMP destination");
                break;
            }

            default:
                throw std::runtime_error(
                    "Unknown instruction at offset " +
                    std::to_string(i - 1));
            }
        }
    }

    void exec(std::vector<std::uint8_t> arr)
    {
        for (int i = 0; i < arr.size(); i++)
        {
            LOG("Current is " + std::to_string(arr[i]));
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
            else if (arr[i] == (uint8_t)COMMANDS::RET_TO_REG)
            {
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                reg[len] = ret;
            }
            else if (arr[i] == (uint8_t)COMMANDS::CALL)
            {
                LOG("[CALL]");
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                ptrdiff_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(ptrdiff_t));
                i += sizeof(ptrdiff_t) - 1;
                void (*f)() = (void (*)())len;
                f();
            }
            else if (arr[i] == (uint8_t)COMMANDS::RUN)
            {
                LOG("[CALL]");
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                ptrdiff_t len;
                uint32_t reallen;
                i++;
                memcpy(&len, arr.data() + i, sizeof(ptrdiff_t));
                i += sizeof(ptrdiff_t);
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t) - 1;
                std::vector<uint8_t> v;
                v.resize(reallen);
                memcpy(v.data(), (void *)len, reallen);
                Machine ma;
                ma.exec(v);
                ret = ma.ret;
            }
            else if (arr[i] == (uint8_t)COMMANDS::RUN)
            {
                LOG("[CALL]");
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                ptrdiff_t len;
                uint32_t reallen;
                i++;
                memcpy(&len, arr.data() + i, sizeof(ptrdiff_t));
                i += sizeof(ptrdiff_t);
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t) - 1;
                reg[len] = reg[reallen];
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
            else if (arr[i] == (uint8_t)COMMANDS::APPEND_REG)
            {
                if (arr.size() - i - 1 < 8)
                    throw std::runtime_error("Invalid APPEND_REG instruction");

                i++;

                uint32_t dest, src;

                memcpy(&dest, arr.data() + i, sizeof(uint32_t));
                i += 4;

                memcpy(&src, arr.data() + i, sizeof(uint32_t));
                i += 3;

                if (dest >= reg.size() || src >= reg.size())
                    throw std::runtime_error("Invalid register index");

                reg[dest].insert(
                    reg[dest].end(),
                    reg[src].begin(),
                    reg[src].end());
            }
            else if (arr[i] == (uint8_t)COMMANDS::REG_TO_BUFF)
            {
                LOG("[REG_TO_BUFF]");
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid REG_TO_BUFF instruction");
                uint32_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t) - 1;
                LOG("[REG_TO_BUFF]:DATA AT REG " + std::to_string(len));
                for (auto i : reg[len])
                {
                    LOG((char)i << ' ');
                }
                std::cout << std::endl;
                buff = reg[len];
                LOG("[REG_TO_BUFF]:DATA AT BUFF");
                for (auto i : buff)
                {
                    LOG((char)i << ' ');
                }
                std::cout << std::endl;
            }
            else if (arr[i] == (uint8_t)COMMANDS::BUFF_TO_REG)
            {
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF_TO_REG instruction");
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
            else if (arr[i] == (uint8_t)COMMANDS::JMP_IF)
            {
                uint32_t len;
                uint32_t num;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                memcpy(&num, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid JMP destination");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data() + i, len);
                i += len;
                uint32_t n;
                memcpy(&n, reg[num].data(), sizeof(uint32_t));
                if (!n)
                {
                    i--;
                    continue;
                }
                if (points.find(res) != points.end())
                {
                    i = points[res] - 1;
                }
                else
                {
                    throw std::runtime_error("Didnt find JMP destination");
                }
            }
            else if (arr[i] == (uint8_t)COMMANDS::JMP_IF_NOT)
            {
                uint32_t len;
                uint32_t num;
                i++;
                memcpy(&len, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                memcpy(&num, arr.data() + i, sizeof(uint32_t));
                i += sizeof(uint32_t);
                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid JMP destination");
                std::string res;
                res.resize(len);
                memcpy(res.data(), arr.data() + i, len);
                i += len;
                uint32_t n;
                memcpy(&n, reg[num].data(), sizeof(uint32_t));
                if (n)
                {
                    i--;
                    continue;
                }
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
            else if (arr[i] == (uint8_t)COMMANDS::GREATER)
            {
                uint32_t first = *((uint32_t *)reg[0].data());
                uint32_t second = *((uint32_t *)reg[1].data());
                uint32_t res = first > second;
                reg[0].clear();
                reg[0].resize(sizeof(uint32_t));
                memcpy(reg[0].data(), &res, sizeof(uint32_t));
            }
            else if (arr[i] == (uint8_t)COMMANDS::LESS)
            {
                uint32_t first = *((uint32_t *)reg[0].data());
                uint32_t second = *((uint32_t *)reg[1].data());
                uint32_t res = first < second;
                reg[0].clear();
                reg[0].resize(sizeof(uint32_t));
                memcpy(reg[0].data(), &res, sizeof(uint32_t));
            }
            else if (arr[i] == (uint8_t)COMMANDS::EQUAL)
            {
                uint32_t first = *((uint32_t *)reg[0].data());
                uint32_t second = *((uint32_t *)reg[1].data());
                uint32_t res = first == second;
                reg[0].clear();
                reg[0].resize(sizeof(uint32_t));
                memcpy(reg[0].data(), &res, sizeof(uint32_t));
            }
        }
    }
};