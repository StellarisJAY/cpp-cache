#ifndef ZSET_H
#define ZSET_H
#include <unordered_map>
#include <string>
#include <optional>

namespace kvstore
{
    constexpr int SKIPLIST_LEVEL_MAX=8;
    class SkipNode
    {
    typedef SkipNode* SkipNodePtr;
    public:
        std::pair<SkipNodePtr, int> levels[SKIPLIST_LEVEL_MAX]; // each level has a span and next pointer
        SkipNode* prev;                                         // previous node of linked-list
        std::string key;
        std::optional<double> score;
    public:
        SkipNode(): score(std::nullopt) {}
        SkipNode(std::string key, double score): key(key), score(std::make_optional(score)) {}
    };

    class SkipList
    {
    private:
        SkipNode* head;
        int size;
    public:
        void put(std::string key, double score);
        void del(std::string key, double score);
    };

    class SortedSet
    {
    private:
        std::unordered_map<std::string, double> map;
    public:
        void put(std::string key, double score);
        std::optional<double> getScore(std::string key);
        void del(std::string key);
    };
}

#endif