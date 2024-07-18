#include <zset.h>

namespace kvstore
{
    void SortedSet::put(std::string key, double score)
    {

    }

    std::optional<double> SortedSet::getScore(std::string key)
    {
        return std::nullopt;
    }

    void SortedSet::del(std::string key)
    {

    }

    int randomLevels()
    {
        return (rand() % SKIPLIST_LEVEL_MAX) + 1;
    }

    void SkipList::put(std::string key, double score)
    {
        auto node = new SkipNode(key, score);
        
    }
    
    void SkipList::del(std::string key, double score)
    {

    }
}