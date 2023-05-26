#pragma once

// #include "../utils.h"
#include "SimpleRuleDefault.h"
#include <algorithm>

namespace zk {
    class SimpleRuleString : public SimpleRuleDefault {
    public:
        std::string value;

        bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = propsMap[id];
                return foundValue == value;
            }
            return false;
        };
        
        bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = propsMap[id];
                return foundValue != value;
            }
            return false;
        };

        static std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
            std::vector<std::string> tokens;
            std::size_t pos = 0;
            std::size_t endPos;

            while ((endPos = str.find(delimiter, pos)) != std::string::npos) {
                tokens.push_back(str.substr(pos, endPos - pos));
                pos = endPos + delimiter.length();
            }

            tokens.push_back(str.substr(pos));

            return tokens;
        }

        bool evaluateIn(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = propsMap[id];
                std::vector<std::string> splits = splitString(value, ", ");

                return std::find(splits.begin(), splits.end(), foundValue) != splits.end();
            }
            return false;
        };

        bool evaluateNotIn(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = propsMap[id];
                std::vector<std::string> splits = splitString(value, ", ");

                return std::find(splits.begin(), splits.end(), foundValue) == splits.end();
            }
            return false;
        };
    };
}