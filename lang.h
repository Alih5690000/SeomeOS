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
    REG,
    PUSH,
    POP,
    REG_TO_BUFF,
    BUFF_TO_REG,
    APPEND_REG,
    CALL
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
            case COMMANDS::PRINT:
            case COMMANDS::ADD:
            case COMMANDS::SUB:
                break;

            case COMMANDS::BUFF:
            {
                uint32_t len = readU32();
                skip(len, "Invalid BUFF data");
                break;
            }

            case COMMANDS::REG:
            {
                uint32_t regIndex = readU32();
                uint32_t len = readU32();

                if (regIndex >= reg.size())
                    throw std::runtime_error("Invalid register index");

                skip(len, "Invalid REG data");
                break;
            }

            case COMMANDS::PUSH:
            case COMMANDS::POP:
            case COMMANDS::REG_TO_BUFF:
            case COMMANDS::BUFF_TO_REG:
            {
                uint32_t regIndex = readU32();

                if (regIndex >= reg.size())
                    throw std::runtime_error("Invalid register index");

                break;
            }

            case COMMANDS::APPEND_REG:
            {
                uint32_t dest = readU32();
                uint32_t src = readU32();

                if (dest >= reg.size() || src >= reg.size())
                    throw std::runtime_error("Invalid register index");

                break;
            }

            case COMMANDS::CALL:
            {
                skip(sizeof(ptrdiff_t), "Invalid CALL instruction");
                break;
            }

            case COMMANDS::POINT:
            {
                uint32_t len = readU32();

                if (len > arr.size() - i)
                    throw std::runtime_error("Invalid POINT name");

                std::string name(
                    reinterpret_cast<const char *>(arr.data() + i),
                    len);

                i += len;
                points[name] = (int)i;
                break;
            }

            case COMMANDS::JMP:
            {
                uint32_t len = readU32();
                skip(len, "Invalid JMP destination");
                break;
            }

            default:
                throw std::runtime_error("Unknown instruction");
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
            else if (arr[i] == (uint8_t)COMMANDS::CALL)
            {
                LOG("[CALL]");
                if (i + 4 >= arr.size())
                    throw std::runtime_error("Invalid BUFF instruction");
                ptrdiff_t len;
                i++;
                memcpy(&len, arr.data() + i, sizeof(ptrdiff_t));
                i += sizeof(ptrdiff_t)-1;
                void (*f)() = (void (*)())len;
                f();
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