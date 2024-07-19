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
        Pattern(char c, PatternType type): data(c), type(type) {}
        Pattern(std::vector<char> vec, PatternType type): data(vec), type(type) {}
        Pattern(): data('*'), type(PatternType::All) {}

        /// @brief match current pattern with char 
        /// @param c 
        /// @return true if matches
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
                    result = std::find(list.begin(), list.end(), c) != list.end();
                    break;
                case PatternType::NotIn:
                    list = std::get<std::vector<char>>(data);
                    result = std::find(list.begin(), list.end(), c) == list.end();
                    break;
            }
            return result;
        }

        /// @brief parse pattern string
        /// @param patStr 
        /// @return pattern list
        static std::vector<Pattern> parse(std::string patStr)
        {
            std::vector<Pattern> patterns;
            for(auto it = patStr.begin(); it != patStr.end();) {
                char c = *it;
                if (c == '*') {
                    patterns.push_back(Pattern('*', PatternType::All));
                    it++;
                    continue;
                }else if (c == '.') {
                    patterns.push_back(Pattern('.', PatternType::Any));
                    it++;
                    continue;
                }else if (c == '[') {
                    // todo IN/NOTIN list
                }else if (c == ']') {

                }else {
                    patterns.push_back(Pattern(c, PatternType::Char));
                    it++;
                    continue;
                }
            }
            return patterns;
        }

        /// @brief match key with pattern list
        /// @param key 
        /// @param patterns 
        /// @return true if matches
        static bool matchKey(std::string key, std::vector<Pattern> patterns)
        {
            int m = key.length();
            int n = patterns.size();
            bool dp[m+1][n+1];
            for(int i = 0; i <= m; i++) {
                for (int j = 0; j <= n; j++) {
                    dp[i][j] = false;
                }
            }
            // dp(i,j) = s[0..i] matches pat[0..j]
            // dp(0,0) = empty string matches empty pattern
            // dp(i,0) = string doesn't match empty pattern
            // dp(0,j) = empty string matches pattern, when pattern is *
            dp[0][0] = true;
            for(int j = 1; j <= n; j++) {
                dp[0][j] = dp[0][j-1] && patterns[j-1].type == PatternType::All;
            }

            // if pat[j-1] not '*': dp(i,j) = dp(i-1,j-1) && match(s[i-1],pat[j-1])
            // else: dp(i,j) = dp(i-1,j) || dp(i,j-1)
            for(int i = 1; i <= m; i++) {
                for(int j = 1; j <= n; j++) {
                    if (patterns[j-1].type == PatternType::All) {
                        dp[i][j] = dp[i-1][j] || dp[i][j-1];
                    }else {
                        dp[i][j] = patterns[j-1].matches(key[i-1]) && dp[i-1][j-1];
                    }
                }
            }
            return dp[m][n];
        }
    };
}
#endif