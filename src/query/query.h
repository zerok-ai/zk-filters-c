#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Rule.h"
#include "QueryType.h"
#include "OperatorType.h"
#include "ConditionType.h"
#include "FieldType.h"

namespace zk {
    static std::unordered_map<std::string, ConditionType> conditionTypeMap = {
        {"AND", AND},
        {"OR", OR},
    };

    static std::unordered_map<std::string, QueryType> queryTypeMap = {
        {"HTTP", HTTP},
        {"MYSQL", MYSQL},
    };

    static std::unordered_map<QueryType, std::string> queryTypeStringMap = {
        {HTTP, "HTTP"},
        {MYSQL, "MYSQL"},
    };

    static std::unordered_map<std::string, OperatorType> operatorTypeMap = {
        {"equals", EQUALS},
        {"not_equals", NOT_EQUALS},
        {"in", IN},
        {"not_in", NOT_IN},
    };

    static std::unordered_map<std::string, FieldType> fieldTypeMap = {
        {"string", STRING},
        {"integer", INTEGER},
        {"key-map", KEY_MAP},
        {"workload-identifier", WORKLOAD_IDENTIFIER}
    };

    class Query{
         public:
            Rule* rule;
            QueryType queryType;
            std::string workloadId;
            std::string traceRole;
            std::string ns;
            std::string service;
    };
}