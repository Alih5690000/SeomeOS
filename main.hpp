#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#ifdef DEBUG
#define LOG(x) std::cout<<x<<std::endl
#else
#define LOG(x)
#endif

template <typename Ellement>
class Map{
    public:
    std::vector<Ellement> v;
    Map(std::vector<Ellement> vec):v(vec){}
    operator std::vector<Ellement>(){
        return v;
    }
    bool Has(std::string n){
        for (auto i:v){
            if (i.name==n) return true;
        }
        return false;
    }
    Ellement Get(std::string n){
        return GetByName(v, n);
    }
    void Set(std::string n, Ellement e){
        for (auto& i:v){
            if (i.name==n){ 
                i=e;
                return;
            }
        }
        throw std::runtime_error("[SetByName]:Couldnt find key "+n);
    }
};

struct Ellement{
    std::string name;
    std::vector<uint8_t> data;
    void WriteInt(uint32_t num){
        data.resize(sizeof(uint32_t));
        memcpy(data.data(), &num, sizeof(uint32_t));
    }
    uint32_t GetInt(){
        uint32_t num;
        memcpy(&num, data.data(), sizeof(uint32_t));
        return num;
    }
    void WriteStr(std::string str){
        data.resize(str.size());
        memcpy(data.data(), str.data(), str.size());
    }
    std::string GetStr(){
        std::string str;
        str.resize(data.size());
        memcpy(str.data(), data.data(), data.size());
        return str;
    }
    void WriteVec(std::vector<Ellement> v){
        size_t size=0;
        for (auto i:v){
            size+=i.data.size()+sizeof(uint32_t);
        }
        data.resize(size);
        size_t offset=0;
        int i=0;
        while (offset<size){
            uint32_t size=v[i].data.size();
            memcpy(data.data()+offset, &size, sizeof(uint32_t));
            offset+=sizeof(uint32_t);
            memcpy(data.data()+offset, v[i].data.data(), 
                v[i].data.size());
            offset+=v[i].data.size();
            i++;
        }
    }
    std::vector<Ellement> GetVec(){
        std::vector<Ellement> res;
        size_t offset=0;
        while (offset<data.size()){
            Ellement e;
            uint32_t size=0;
            memcpy(&size, data.data()+offset, sizeof(uint32_t));
            offset+=sizeof(uint32_t);
            e.data.resize(size);
            memcpy(e.data.data(), data.data()+offset, size);
            offset+=size;
            res.push_back(e);
        }
        return res;
    }
    void WriteMap(const std::vector<Ellement>& m){
        size_t size=0;
        for (auto i:m){
            size+=i.name.size();
            size+=i.data.size();
            size+=sizeof(uint32_t)*2;
        }
        data.resize(size);
        size_t offset=0;
        int i=0;
        while (offset<size){
            uint32_t nameSize=m[i].name.size();
            memcpy(data.data()+offset, &nameSize, sizeof(uint32_t));
            offset+=sizeof(uint32_t);
            memcpy(data.data()+offset, m[i].name.data(), nameSize);
            offset+=nameSize;
            uint32_t dataSize=m[i].data.size();
            memcpy(data.data()+offset, &dataSize, sizeof(uint32_t));
            offset+=sizeof(uint32_t);
            memcpy(data.data()+offset, m[i].data.data(), dataSize);
            offset+=dataSize;
            i++;
        }
    }
    Map<Ellement> GetMap(){
        std::vector<Ellement> res;
        size_t offset=0;
        while (offset<data.size()){
            Ellement e;
            uint32_t nameSize;
            memcpy(&nameSize, data.data()+offset, sizeof(uint32_t));
            offset+=sizeof(uint32_t);
            e.name.resize(nameSize);
            memcpy(e.name.data(), data.data()+offset, nameSize);
            offset+=nameSize;
            uint32_t dataSize;
            memcpy(&dataSize, data.data()+offset, sizeof(uint32_t));
            offset+=sizeof(uint32_t);
            e.data.resize(dataSize);
            memcpy(e.data.data(), data.data()+offset, dataSize);
            offset+=dataSize;
            res.push_back(e);
        }
        return res;
    }
    Ellement GetByName(std::string n){
        for (auto i:GetMap().v){
            if (i.name==n) return i;
        }
        throw std::runtime_error("[GetByName]:Couldnt find key "+n);
    }
    Ellement(uint32_t num){
        WriteInt(num);
    }
    Ellement(std::string k,uint32_t num){
        WriteInt(num);
        name=k;
    }
    Ellement(std::string num){
        WriteStr(num);
    }
    Ellement(std::string k,std::string num){
        WriteStr(num);
        name=k;
    }
    Ellement(std::vector<Ellement> num){
        WriteVec(num);
    }
    Ellement(std::string k,std::vector<Ellement> num){
        WriteVec(num);
        name=k;
    }
    Ellement(std::vector<Ellement> v, bool){
        WriteMap(v);
    }
    Ellement(std::string k,std::vector<Ellement> v, bool){
        WriteMap(v);
        name=k;
    }
    Ellement()=default;
};

Ellement GetByName(std::vector<Ellement> v, std::string n){
    for (auto i:v){
        if (i.name==n) return i;
    }
    throw std::runtime_error("[GetByName]:Couldnt find key "+n);
}

/**
 * @brief Just a database. Look at methods brief if you want to know more.
 */
class DataBase{
    public:
    /**
     * @brief Just fields of database.
     * @note Storing types with pointer as field or with vtable is NOT RECOMENDED
     */
    std::map<std::string,std::vector<std::uint8_t>> attrs;
    /**
     * @brief Constructor with path to database.
     * @param path Path to database.
     */
    DataBase(const std::string& path){
        std::ifstream file(path,std::ios::binary);
        while (file.peek() != std::ifstream::traits_type::eof()){
            uint32_t lengthOfName;
            file.read((char*)&lengthOfName,sizeof(uint32_t));
            LOG("Length of name is "+std::to_string(lengthOfName));
            std::string name;
            name.resize(lengthOfName);
            file.read(name.data(),lengthOfName);
            LOG("Name is "+name);
            uint32_t lengthOfData;
            file.read((char*)&lengthOfData,sizeof(uint32_t));
            LOG("Length of data is "+std::to_string(lengthOfData));
            std::vector<uint8_t> data;
            data.resize(lengthOfData);
            file.read((char*)data.data(),lengthOfData);
            attrs[name]=data;
        }
    }

    /**
     * @brief Default constructor with no parameters.
     */
    DataBase()=default;
    /**
     * @brief Dumps whole database to file.
     * @param path Path to database.
     */
    void WriteTo(const std::string& path){
        std::ofstream file(path,std::ios::binary | std::ios::trunc);
        for (auto [k,v]:attrs){
            uint32_t size=k.size();
            LOG("[WRITE]Length of name is "+std::to_string(size));
            file.write((char*)&size,sizeof(uint32_t));
            file.write(k.data(),size);
            LOG("[WRITE]Name is "+k);
            uint32_t size2=v.size();
            LOG("[WRITE]Length of data is "+std::to_string(size2));
            file.write((char*)&size2,sizeof(uint32_t));
            char* data=(char*)v.data();
            file.write(data,size2);
        }
    }
    /**
     * @brief Adds integer to DataBase::atrs eventually turning it into vector of uint8_t
     * @param key Key.
     * @param num Integer to be stored.
     */
    void AddInt(const std::string& key, uint32_t num){
        std::vector<uint8_t> arr;
        arr.resize(4);
        memcpy(arr.data(), &num, sizeof(uint32_t));
        attrs[key]=std::move(arr);
    }
    /**
     * @brief Adds string to DataBase::atrs eventually turning it into vector of uint8_t
     * @param key Key.
     * @param str String to be stored.
     */
    void AddStr(const std::string& key, const std::string& str){
        std::vector<uint8_t> arr;
        arr.resize(str.size());
        memcpy(arr.data(), str.data(), str.size());
        attrs[key]=std::move(arr);
    }
    /**
     * @brief Adds vector to DataBase::atrs eventually turning it into vector of uint8_t
     * @param key Key.
     * @param vec Vector to be stored.
     * @param T Template parameter which is type of a vector.
     */
    template <typename T>
    void AddVector(const std::string& key, const std::vector<T>& vec){
        std::vector<uint8_t> arr;
        arr.resize(vec.size()*sizeof(T));
        memcpy(arr.data(), vec.data(), vec.size()*sizeof(T));
        attrs[key]=std::move(arr);
    }
    /**
     * @brief Reads bytes and converts it to std::vector.
     * @param key Key.
     * @param T Type of std::vector
     * @return Vector with T type
     */
    template <typename T>
    std::vector<T> ReadVector(const std::string& key){
        std::vector<T> l(attrs[key].size()/sizeof(T));
        memcpy(l.data(), attrs[key].data(), attrs[key].size());
        return l;
    }
    void AddVectorEx(const std::string& key, std::vector<Ellement> vec){
        Ellement ell;
        ell.WriteVec(vec);
        attrs[key]=ell.data;
    }
    std::vector<Ellement> ReadVectorEx(const std::string& key){
        Ellement e;
        e.data=attrs[key];
        return e.GetVec();
    }
    void AddMapEx(const std::string& key, std::vector<Ellement> vec){
        Ellement ell;
        ell.WriteMap(vec);
        attrs[key]=ell.data;
    }
    Map<Ellement> ReadMapEx(const std::string& key){
        Ellement e;
        e.data=attrs[key];
        return e.GetMap();
    }
    /**
     * @brief Adds map to DataBase::atrs eventually turning it into vector of uint8_t
     * @param key Key.
     * @param map Map to be stored.
     * @param K Type of key
     * @param V Type of value
     */
    template <typename K, typename V>
    void AddMap(const std::string& key, std::map<K,V> map){
        std::vector<uint8_t> arr;
        arr.resize(map.size()*(sizeof(K)+sizeof(V)));
        size_t offset=0;
        for (const auto& [k,v]:map){
            memcpy(arr.data()+offset, &k, sizeof(K));
            offset+=sizeof(K);
            memcpy(arr.data()+offset, &v, sizeof(V));
            offset+=sizeof(V);
        }
        attrs[key]=std::move(arr);
    }
    /**
     * @brief Reads bytes and converts it to std::vector.
     * @param key Key.
     * @param K Type of key
     * @param V Type of value
     */
    template <typename K, typename V>
    std::map<K,V> ReadMap(const std::string& key){
        const std::vector<uint8_t>& arr=attrs[key];
        std::map<K,V> res;
        int i=0;
        while(i<arr.size()){
            K k;
            memcpy(&k, arr.data()+i, sizeof(K));
            i+=sizeof(K);
            V v;
            memcpy(&v, arr.data()+i, sizeof(V));
            i+=sizeof(V);
            res[k]=v;
        }
        return res;
    }
};

