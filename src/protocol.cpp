#include <protocol.h>
#include <sstream>
#include <assert.h>
#include <iostream>
namespace kvstore
{

    int numCharLength(int num) {
        int len = 0;
        while(num > 0) {
            num /= 10;
            len++;
        }
        return len;
    }

    std::optional<RESPCommand> decodeBulk(std::string buf)
    {
        try {
            std::string::iterator it = buf.begin();
            for (; it != buf.end(); it++) {
                if (*it == '\r' && *(it+1) == '\n') {
                    break;
                }
            }
            if (it == buf.end()) return std::nullopt;
            auto num = buf.substr(1, it - buf.begin());
            int length = std::stoi(num, 0, 10);
            it = it+2;
            auto bulk = buf.substr(it - buf.begin(), length);
            return std::make_optional<RESPCommand>(bulk, 1);
        }catch (std::invalid_argument e) {
            return std::nullopt;
        }
        return std::nullopt;
    }

    std::optional<RESPCommand> decodeArray(std::string buf)
    {
        std::vector<RESPCommand> *array = new std::vector<RESPCommand>();
        try {
            std::string::iterator it = buf.begin();
            for (; it != buf.end(); it++) {
                if (*it == '\r' && *(it+1) == '\n') {
                    break;
                }
            }
            if (it == buf.end()) return std::nullopt;
            auto num = buf.substr(1, it-buf.begin());
            int count = std::stoi(num);
            it = it+2;
            std::optional<RESPCommand> nested;
            int nestedLength = 0;
            for (int i = 0; i < count; i++) {
                switch (*it) {
                case BULK:
                    nested = decodeBulk(buf.substr(it-buf.begin(), buf.end()-it));
                    if (!nested.has_value()) return std::nullopt;
                    nestedLength = nested.value().getData<std::string>().value().length();
                    it += nestedLength + 4 + numCharLength(nestedLength) + 1;
                    array->push_back(nested.value());
                    break;
                case INT:
                case ERROR:
                case SIMPLE://fallthrough
                    return std::nullopt;
                }
            }
            return std::make_optional<RESPCommand>(array);
        }catch(std::exception e) {
            return std::nullopt;
        }
        return std::nullopt;
    }


    std::string RESPCommand::toString()
    {
        std::stringstream stream;
        const auto CRLF = "\r\n";
        switch (getType()) {
        case NIL:
            stream << "$" << -1 << CRLF;
            break;
        case SIMPLE:
            stream << (char)getType() << CRLF << getData<std::string>().value() << CRLF;
            break;
        case BULK:
            stream << (char)getType() << getData<std::string>().value().length() << CRLF << getData<std::string>().value() << CRLF;
            break;
        case ERROR:
            stream << (char)getType() << getData<std::string>().value() << CRLF;
            break;
        case INT:
            stream << (char)getType() << getData<int>().value() << CRLF;
            break;
        case ARRAY:
            auto array = getData<std::vector<RESPCommand>*>().value();
            stream << (char)getType() << array->size() << CRLF;
            for (auto it = array->begin(); it != array->end(); it++) {
                stream << (*it).toString();
            }
            break;
        }
        return stream.str();
    }

    std::optional<RESPCommand> RedisCodec::decode(std::string buf)
    {
        if (buf.length() < 3) {
            return std::nullopt;
        }
        switch (buf.at(0)) {
        case SIMPLE:
        case BULK:
            return decodeBulk(buf);
        case INT:
        case ARRAY:
            return decodeArray(buf);
        default:
            return std::nullopt;
        }
    }

    std::string RedisCodec::encode(RESPCommand& command)
    {
        auto res = command.toString();
        return res;
    }
}