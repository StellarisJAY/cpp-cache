#ifndef LINK_LIST_H
#define LINK_LIST_H
#include <db.h>
#include <protocol.h>

namespace kvstore 
{
    void handleLPush(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response);
    void handleRPush(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response);
    void handleLPop(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response);
    void handleRPop(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response);
    void handleLRange(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response);
    void handleLIndex(Database *db, int argc, std::vector<RESPCommand> argv, std::shared_ptr<ClientConn> conn, RESPCommand& response);
}

#endif