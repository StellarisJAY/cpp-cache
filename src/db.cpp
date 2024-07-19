#include <db.h>
#include <protocol.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include <exception>
#include <iostream>
#include "list.h"
#include "pattern.h"

#define REG_COMMAND(name, func) \
        handlers.insert(std::make_pair<std::string, cmdHandler>(std::string(name), func));

namespace kvstore
{
    Database::Database()
    {
        init();
    }

    Database::~Database()
    {

    }

    Entry& Database::getEntry(int idx, std::string key)
    {
        if (idx > 16 || idx < 0) {
            throw new std::out_of_range("db idx out of range");
        }
        auto expireAt = expireDicts[idx][key];
        auto now = std::chrono::system_clock::now();
        if (expireAt.time_since_epoch() == std::chrono::seconds(0) || expireAt.time_since_epoch() > now.time_since_epoch()) {
            return dicts[idx][key];
        } else {
            delEntry(idx, key);
            return dicts[idx][key];
        }
    }

    void Database::putEntry(int idx, std::string key, std::variant<std::string, List, Hash, Set, ZSet> value, DataType type)
    {
        if (idx > 16 || idx < 0) {
            throw new std::out_of_range("db idx out of range");
        }
        dicts[idx].insert_or_assign(key, Entry(type, value));
    }

    void Database::handleCommand(RESPCommand& req, RESPCommand& resp, std::shared_ptr<ClientConn> conn)
    {
        auto array = req.getData<std::vector<RESPCommand>*>().value();
        if (array->size() == 0) {
            resp.setError(ErrUnknownCommand);
            return;
        }
        std::string cmdName = (*array)[0].getData<std::string>().value();
        auto handler = this->handlers[cmdName];
        if (!handler) {
            resp.setOK();
            return;
        }
        handler(this, array->size(), *array, conn, resp);
    }

    int Database::dbSize(int idx)
    {
        if (idx > 16 || idx < 0) {
            throw new std::out_of_range("db idx out of range");
        }
        return dicts[idx].size();
    }

    void Database::delEntry(int idx, std::string key)
    {
        if (idx > 16 || idx < 0) {
            throw new std::out_of_range("db idx out of range");
        }
        auto entry = dicts[idx][key];
        dicts[idx].erase(key);
        expireDicts[idx].erase(key);
        if (entry.type == LIST) {
            List list = std::get<List>(entry.data);
            delete list;
        }else if (entry.type == HASH) {
            Hash hash = std::get<Hash>(entry.data);
            delete hash;
        }else if (entry.type == SET) {
            Set set = std::get<Set>(entry.data);
            delete set;
        }
    }

    std::optional<std::chrono::time_point<std::chrono::system_clock>> Database::getTTL(int idx, std::string key) 
    {
        if (idx < 0 || idx >= 16) throw new std::out_of_range("db idx out of range");
        auto expireAt = expireDicts[idx][key];
        if (expireAt.time_since_epoch() == std::chrono::seconds(0)) {
            return std::nullopt;
        }
        return std::make_optional(expireAt);
    }

    void Database::setExpire(int idx, std::string key, int seconds)
    {
        if (idx < 0 || idx >= 16) throw new std::out_of_range("db idx out of range");
        auto expire = std::chrono::system_clock::now();
        auto duration = std::chrono::seconds(seconds);
        expire += duration;
        expireDicts[idx].insert_or_assign(key, expire);
    }

    std::unordered_map<std::string, Entry> *Database::getDict(int idx) {
        if (idx < 0 || idx >= 16) throw new std::out_of_range("db idx out of range");
        return &dicts[idx];
    }

    void handleSet(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc < 3) {
            response.setError(ErrWrongNumberOfArgs);
        }
        auto key = argv[1].getData<std::string>().value();
        auto value = argv[2].getData<std::string>().value();
        auto entry = db->getEntry(conn->selectedDB, key);
        if (entry.type == EMPTY || entry.type == RAW) {
            db->putEntry(conn->selectedDB, key, value, RAW);
            response.setOK();
        }else {
            response.setError(ErrWrongType);
        }
    }
    
    void handleGet(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 1) {
            response.setError(ErrWrongNumberOfArgs);
        }
        auto key = argv[1].getData<std::string>().value();
        auto entry = db->getEntry(conn->selectedDB, key);
        if (entry.type == EMPTY) {
            response.setType(NIL);
            return;
        }
        if (entry.type == RAW) {
            auto value = std::get<std::string>(entry.data);
            response.setData(value);
            response.setType(BULK);
        }else {
            response.setError(ErrWrongType);
        }
    }

    void handleDBSize(Database *db, 
                      int argc, 
                      std::vector<RESPCommand> argv, 
                      std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        response.setInt(db->dbSize(conn->selectedDB));
    }

    void handleStrlen(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 1) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto entry = db->getEntry(conn->selectedDB, argv[1].getData<std::string>().value());
        if (entry.type == RAW) {
            response.setInt(std::get<std::string>(entry.data).length());
            return;
        } else if(entry.type == EMPTY) {
            response.setInt(0);
        } else {
            response.setError(ErrWrongType);
        }
    }

    void handleDel(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 1) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        int deleted = 0;
        for (auto it = argv.begin()+1; it != argv.end(); it++) {
            auto key = it->getData<std::string>().value();
            auto entry = db->getEntry(conn->selectedDB, key);
            if (entry.type == EMPTY) {
                continue;
            }
            db->delEntry(conn->selectedDB, key);
            deleted++;
        }
        response.setInt(deleted);
    }

    void handleExists(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 1) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        int exists = 0;
        for (auto it = argv.begin() + 1; it != argv.end(); it++) {
            auto key = it->getData<std::string>().value();
            if (db->getEntry(conn->selectedDB,key).type != EMPTY) {
                exists++;
            }
        }
        response.setInt(exists);
    }

    void handleType(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 1) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto key = argv[1].getData<std::string>().value();
        response.setType(BULK);
        switch (db->getEntry(conn->selectedDB, key).type) {
        case RAW:
            response.setData(std::string("string"));
            break;
        case LIST:
            response.setData(std::string("list"));
            break;
        case HASH:
            response.setData(std::string("hash"));
            break;
        case SET:
            response.setData(std::string("set"));
            break;
        case ZSET:
            response.setData(std::string("zset"));
            break;
        default:
            response.setData(std::string("none"));
        }
    }

    void handleTTL(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc != 2) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto key = argv[1].getData<std::string>().value();
        auto expireOptional = db->getTTL(conn->selectedDB, key);
        if (expireOptional.has_value()) {
            auto now = std::chrono::system_clock::now();
            auto expireAt = expireOptional.value();
            if (now >= expireAt) {
                response.setInt(0);
                db->delEntry(conn->selectedDB, key);
                return;
            }
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(expireOptional.value() - now);
            response.setInt(duration.count());
        }else {
            response.setInt(-2);
        }
    }

    void handleExpire(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc != 3) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto key = argv[1].getData<std::string>().value();
        auto secondsStr = argv[2].getData<std::string>().value();
        try {
            auto seconds = std::stoi(secondsStr);
            if (db->getEntry(conn->selectedDB, key).type == EMPTY) {
                response.setInt(0);
                return;
            }
            db->setExpire(conn->selectedDB, key, seconds);
            response.setInt(1);
        }catch(std::invalid_argument const& e) {
            response.setError(ErrNotIntegerOrOutOfRange);
        }catch(std::out_of_range const& e) {
            response.setError(ErrNotIntegerOrOutOfRange);
        }
    }

    void handleKeys(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        auto dict = db->getDict(conn->selectedDB);
        if (dict->size() == 0) {
            response.setEmptyArray();
            return;
        }
        auto patStr = argv[1].getData<std::string>().value();
        auto result = new std::vector<RESPCommand>();
        std::vector<Pattern> patterns = Pattern::parse(patStr);
        for(auto it = dict->begin(); it != dict->end(); it++) {
            std::string key = it->first;
            if (Pattern::matchKey(key, patterns)) {
                result->push_back(RESPCommand(key, true));
            }
        }
        response.setType(ARRAY);
        response.setData(result);
    }
    
    void handleCOMMAND(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        response.setOK();
    }

    void handleSelect(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc != 2) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto idxStr = argv[1].getData<std::string>().value();
        int idx = atoi(idxStr.c_str());
        if (idx < 0 || idx >= 16) {
            response.setError(ErrDbIdxOutOfRange);
            return;
        }
        conn->selectedDB = idx;
        response.setOK();
    }

    void handleFlushDB(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        int dbIdx = conn->selectedDB;
        auto dict = db->getDict(dbIdx);
        dict->clear();
        response.setOK();
    }

    void Database::init()
    {
        REG_COMMAND("set", handleSet);
        REG_COMMAND("get", handleGet);

        REG_COMMAND("lpush", handleLPush);
        REG_COMMAND("rpush", handleRPush);
        REG_COMMAND("lpop", handleLPop);
        REG_COMMAND("rpop", handleRPop);
        REG_COMMAND("lindex", handleLIndex);
        REG_COMMAND("lrange", handleLRange);
        REG_COMMAND("llen", handleLLen);

        REG_COMMAND("strlen", handleStrlen);
        REG_COMMAND("dbsize", handleDBSize);
        REG_COMMAND("del", handleDel);
        REG_COMMAND("exists", handleExists);
        REG_COMMAND("ttl", handleTTL);
        REG_COMMAND("expire", handleExpire);
        REG_COMMAND("keys", handleKeys);
        REG_COMMAND("select", handleSelect);
        REG_COMMAND("flushdb", handleFlushDB);

        REG_COMMAND("command", handleCOMMAND);
    }
}