#ifndef KEYS_PATTERN_H
#define KEYS_PATTEN_H
#include <variant>
#include <vector>
#include <algorithm>
#include <string>

namespace kvstore 
{
    enum PatternType
    {
        Char = 0,
        Any = 1,
        All = 2,
        In = 3,
        NotIn = 4,
    };

    class Pattern
    {
    private:
        PatternType type;
        std::variant<char,std::vector<char>> data;
    public:
        Pattern(char c): data(c), type(PatternType::Char) {}
        Pattern(std::vector<char> vec, PatternType type): data(vec), type(type) {}
        bool matches(char c) 
        {
            bool result = false;
            std::vector<char> list;
            switch(this->type) {
                case PatternType::Char:
                    result = c == std::get<char>(data);
                    break;
                case PatternType::Any:
                    result = true;
                    break;
                case PatternType::In:
                    list = std::get<std::vector<char>>(data);
                    result = std::find(list.begin(), list.end() + 1, c) != list.end() + 1;
                    break;
                case PatternType::NotIn:
                    list = std::get<std::vector<char>>(data);
                    result = std::find(list.begin(), list.end() + 1, c) == list.end() + 1;
                    break;
            }
            return result;
        }
    };
}
#endif