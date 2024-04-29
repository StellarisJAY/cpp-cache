#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <sstream>
#include <optional>
#include <variant>
#include <vector>

namespace kvstore
{

    enum RESPCommandType 
    {
        SIMPLE='+',
        BULK='$',
        INT=':',
        ERROR='-',
        ARRAY='*',
        NIL='@',
    };
    typedef std::string RedisError;
    class RESPCommand
    {
    private:
        RESPCommandType type;
        std::variant<int, std::string, std::vector<RESPCommand>> data;
    public:
        template <typename T>
        std::optional<T> getData()
        {
            switch (type) {
            case INT:
                if (std::is_same<T, int>::value) return std::make_optional(std::get<T>(data));
                break;
            case SIMPLE:
                if (std::is_same<T, std::string>::value) return std::make_optional(std::get<T>(data));
                break;
            case BULK:
                if (std::is_same<T, std::string>::value) return std::make_optional(std::get<T>(data));
                break;
            case ARRAY:
                if (std::is_same<T, std::vector<RESPCommand>>::value) return std::make_optional(std::get<T>(data));
                break;
            case ERROR:
                if (std::is_same<T, std::string>::value) return std::make_optional(std::get<T>(data));
                break;
            }
            return std::nullopt;
        }
        
        RESPCommandType getType()
        {
            return type;
        }
        RESPCommand(int data): data(data), type(INT) {}
        RESPCommand(std::string data, bool bulk): data(data), type(bulk?BULK:SIMPLE) {}
        RESPCommand(std::vector<RESPCommand> data): data(data), type(ARRAY) {}
        RESPCommand(RedisError error): data(error), type(ERROR) {}
        RESPCommand(){}
        std::string toString();
        void setType(RESPCommandType type) {this->type = type;}
        void setData(std::variant<int, std::string, std::vector<RESPCommand>> data) {this->data = data;}
        void setError(RedisError error) {
            type = ERROR;
            data = error;
        }
        void setOK() {
            type = BULK;
            data = "OK";
        }
        void setInt(int data)
        {
            type = INT;
            this->data = data;
        }
        void setEmptyArray()
        {
            type = ARRAY;
            data = std::vector<RESPCommand>();
        }
    };

    class RedisCodec
    {
    public:
        std::optional<RESPCommand> decode(std::string buf);
        std::string encode(RESPCommand &command);
    };

    static RedisError ErrUnknownCommand = std::string("ERR unknown command");
    static RedisError ErrWrongType = std::string("WRONGTYPE Operation against a key holding the wrong kind of value");
    static RedisError ErrWrongNumberOfArgs = std::string("ERR wrong number of arguments for command");
    static RedisError ErrNotIntegerOrOutOfRange = std::string("ERR value is not an integer or out of range");
}

#endif