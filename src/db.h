#ifndef DB_H
#define DB_H
#include <unordered_map>
#include <string>
#include <vector>
#include <list>
#include <protocol.h>
#include <variant>
#include <unordered_set>
#include <functional>
#include <conn.h>
#include <memory>
#include <chrono>
#include <zset.h>

namespace kvstore
{
    enum DataType
    {
        RAW,
        LIST,
        HASH,
        SET,
        ZSET,
        EMPTY,
    };

    typedef std::list<std::string>* List;
    typedef std::unordered_map<std::string,std::string>* Hash;
    typedef std::unordered_set<std::string>* Set;
    typedef kvstore::SortedSet* ZSet;
    class Entry
    {
    public:
        Entry(): type(EMPTY) {};
        Entry(DataType type, std::variant<std::string, List, Hash, Set, ZSet> data): type(type), data(data) {}
        DataType type;
        std::variant<std::string, List, Hash, Set, ZSet> data;
    };

    class Database
    {
    typedef std::function<void(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)> cmdHandler;
    public:
        Database();
        ~Database();
        void init();
        void handleCommand(RESPCommand &req, RESPCommand &resp, std::shared_ptr<ClientConn> conn);
        Entry& getEntry(int idx, std::string key);
        void putEntry(int idx, std::string key, std::variant<std::string, List, Hash, Set, ZSet> data, DataType type);
        void delEntry(int idx, std::string key);
        int dbSize(int idx);
        std::optional<std::chrono::time_point<std::chrono::system_clock>> getTTL(int idx, std::string key);
        void setExpire(int idx, std::string key, int seconds);
        std::unordered_map<std::string, Entry> *getDict(int idx);
    private:
        std::unordered_map<std::string, Entry> dicts[16];
        std::unordered_map<std::string, cmdHandler> handlers;
        std::unordered_map<std::string, std::chrono::time_point<std::chrono::system_clock>> expireDicts[16];
    };
}

#endif