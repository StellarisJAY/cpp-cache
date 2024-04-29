#include "list.h"
#include <list>
#include <string>

namespace kvstore
{
    void handleLPush(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response) 
    {
        if (argc <= 2) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto lKey = argv[1].getData<std::string>().value();
        auto entry = db->getEntry(conn->selectedDB, lKey);
        if (entry.type == EMPTY) {
            List list = new std::list<std::string>();
            for (auto it = argv.begin() + 2; it != argv.end(); it++) {
                list->push_front(it->getData<std::string>().value());
            }
            db->putEntry(conn->selectedDB, lKey, list, LIST);
            response.setInt(list->size());
        }else if (entry.type == LIST) {
            List list = std::get<List>(entry.data);
            for (auto it = argv.begin() + 2; it != argv.end(); it++) {
                list->push_front(it->getData<std::string>().value());
            }
            response.setInt(list->size());
        }else {
            response.setError(ErrWrongType);
        }
    }

    void handleRPush(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 2) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto lKey = argv[1].getData<std::string>().value();
        auto entry = db->getEntry(conn->selectedDB, lKey);
        if (entry.type == EMPTY) {
            List list = new std::list<std::string>();
            for (auto it = argv.begin() + 2; it != argv.end(); it++) {
                list->push_back(it->getData<std::string>().value());
            }
            db->putEntry(conn->selectedDB, lKey, list, LIST);
            response.setInt(list->size());
        }else if (entry.type == LIST) {
            List list = std::get<List>(entry.data);
            for (auto it = argv.begin() + 2; it != argv.end(); it++) {
                list->push_back(it->getData<std::string>().value());
            }
            response.setInt(list->size());
        }else {
            response.setError(ErrWrongType);
        }
    }

    void handleLPop(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 1) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto lKey = argv[1].getData<std::string>().value();
        auto entry = db->getEntry(conn->selectedDB, lKey);
        if (entry.type == EMPTY) {
            response.setType(NIL);
        }else if (entry.type == LIST) {
            List list = std::get<List>(entry.data);
            if (list->size() == 0) {
                response.setType(NIL);
            }else {
                response.setType(BULK);
                auto data = list->front();
                list->pop_front();
                response.setData(data);
            }
        }else {
            response.setError(ErrWrongType);
        }
    }

    void handleRPop(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc <= 1) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto lKey = argv[1].getData<std::string>().value();
        auto entry = db->getEntry(conn->selectedDB, lKey);
        if (entry.type == EMPTY) {
            response.setType(NIL);
        }else if (entry.type == LIST) {
            List list = std::get<List>(entry.data);
            if (list->size() == 0) {
                response.setType(NIL);
            }else {
                response.setType(BULK);
                auto data = list->back();
                list->pop_back();
                response.setData(data);
            }
        }else {
            response.setError(ErrWrongType);
        }
    }

    void handleLRange(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc != 4) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto key = argv[1].getData<std::string>().value();
        auto startStr = argv[2].getData<std::string>().value();
        auto endStr = argv[3].getData<std::string>().value();

        try {
            auto start = std::stoi(startStr, 0, 10);
            auto end = std::stoi(endStr, 0, 10);
            auto entry = db->getEntry(conn->selectedDB, key);
            if (entry.type == EMPTY) {
                response.setEmptyArray();
            }else if (entry.type == LIST) {
                List list = std::get<List>(entry.data);
                if ((start > 0 && start >= list->size()) || (start < 0 && abs(start)-1 >= list->size())) {
                    response.setEmptyArray();
                    return;
                }
                if ((end > 0 && end >= list->size()) || (end < 0 && abs(end)-1 >= list->size())) {
                    response.setEmptyArray();
                    return;
                }
                if (start < 0) {
                    start = list->size() + start;
                }
                if (end < 0) {
                    end = list->size() + end;
                }
                if (start > end) {
                    response.setEmptyArray();
                    return;
                }
                auto it = list->begin();
                auto result = std::vector<RESPCommand>();
                for (int i = 0; i < start; i++) {
                    it++;
                }
                for (; it != list->end() && start <= end; it++) {
                    result.push_back(RESPCommand(*it, true));
                    start++;
                }
                response.setType(ARRAY);
                response.setData(result);
            }else {
                response.setError(ErrWrongType);
            }
        }catch(std::invalid_argument e) {
            response.setError(ErrNotIntegerOrOutOfRange);
        }catch(std::out_of_range e) {
            response.setError(ErrNotIntegerOrOutOfRange);
        }
    }

    void handleLIndex(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response)
    {
        if (argc != 3) {
            response.setError(ErrWrongNumberOfArgs);
            return;
        }
        auto key = argv[1].getData<std::string>().value();
        auto idxStr = argv[2].getData<std::string>().value();
        try {
            int idx = std::stoi(idxStr, 0, 10);
            auto entry = db->getEntry(conn->selectedDB, key);
            if (entry.type == LIST) {
                List list = std::get<List>(entry.data);
                if ((idx > 0 && idx >= list->size()) || (idx < 0 && abs(idx)-1 >= list->size())) {
                    response.setType(NIL);
                    return;
                }
                response.setType(BULK);
                if (idx < 0) {
                    idx = list->size()+idx;
                }
                auto it = list->begin();
                for (; it != list->end() && idx > 0; it++) idx--;
                response.setData(*it);
            }else if (entry.type == EMPTY) {
                response.setType(NIL);
            }else {
                response.setError(ErrWrongType);
            }
        }catch(std::invalid_argument e) {
            response.setError(ErrNotIntegerOrOutOfRange);
        }catch(std::out_of_range e) {
            response.setError(ErrNotIntegerOrOutOfRange);
        }
    }
}