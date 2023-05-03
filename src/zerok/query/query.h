#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <any>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/pointer.h"
#include "rapidjson/writer.h"

namespace zk {
    enum class ConditionType{
        AND,
        OR
    };

    std::unordered_map<std::string, ConditionType> conditionTypeMap = {
        {"AND", ConditionType::AND},
        {"OR", ConditionType::OR},
    };

    enum class QueryType{
        HTTP,
        MYSQL
    };

    std::unordered_map<std::string, QueryType> queryTypeMap = {
        {"HTTP", QueryType::HTTP},
        {"MYSQL", QueryType::MYSQL},
    };

    enum class OperatorType{
        EQUALS, 
        NOT_EQUALS
    };

    std::unordered_map<std::string, OperatorType> operatorTypeMap = {
        {"equals", OperatorType::EQUALS},
        {"not_equals", OperatorType::NOT_EQUALS},
    };

    enum class FieldType{
        STRING, 
        INTEGER,
        KEY_MAP,
        WORKLOAD_IDENTIFIER
    };

    std::unordered_map<std::string, FieldType> fieldTypeMap = {
        {"string", FieldType::STRING},
        {"integer", FieldType::INTEGER},
        {"key-map", FieldType::KEY_MAP},
        {"workload-identifier", FieldType::WORKLOAD_IDENTIFIER}
    };


    class Rule{
        public:
            virtual ~Rule() = default;
            virtual bool evaluate(std::map<std::string, std::string> propsMap) const = 0;
    };

    class CompositeRule : public Rule {
         public:
            ConditionType condition;
            std::vector<Rule*> rules;

            bool evaluate(std::map<std::string, std::string> propsMap) const override{
                if(condition == ConditionType::AND){
                    for (Rule* rule : rules) {
                        bool evaluationResult = rule->evaluate(propsMap);
                        if(!evaluationResult){
                            return false;
                        }
                    }

                    return true;
                }
                else if(condition == ConditionType::OR){
                    for (Rule* rule : rules) {
                        bool evaluationResult = rule->evaluate(propsMap);
                        if(evaluationResult){
                            return true;
                        }
                    }

                    return false;
                }
                
                return false;
            }
    };

    class Query{
         public:
            Rule* rule;
            QueryType queryType;
    };

    class SimpleRule : public Rule {
        public:
            std::string id;
            // std::string field;
            std::string key;
            FieldType type;
            std::string input;
            OperatorType operatorType;

            bool evaluate(std::map<std::string, std::string> propsMap) const override{
                switch (operatorType)
                {
                    case OperatorType::EQUALS:
                        return evaluateEquals(propsMap);
                    
                    case OperatorType::NOT_EQUALS:
                        return evaluateNotEquals(propsMap);
                    
                    default:
                        break;
                }

                return false;
            }

            virtual bool evaluateEquals(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const = 0;
    };

    class SimpleRuleDefault : public SimpleRule {
        public:
            bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
                return false;
            };
            bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
                return false;
            };
    };

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
    };

    class SimpleRuleKeyValue : public SimpleRuleDefault {
        public:
            std::string value;

            std::string extractValue(std::map<std::string, std::string> propsMap) const {
                if(propsMap.count(id)){
                    const std::string json = propsMap[id];
                    const char* jsonCstr = json.c_str();
                    rapidjson::Document doc;
                    doc.Parse(jsonCstr);

                    const char* keyCstr = key.c_str();
                    //https://rapidjson.org/md_doc_pointer.html
                    rapidjson::Pointer pointer(keyCstr);

                    // Extract the value using JSONPath
                    if (!pointer.IsValid()) {
                        return "ZK_NULL";
                    }

                    const rapidjson::Value* result = pointer.Get(doc);
                    std::string foundValue = result->GetString();

                    return foundValue;
                }

                return "ZK_NULL";
            }

            bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
                std::string foundValue = extractValue(propsMap);
                if (foundValue != "ZK_NULL"){
                    return foundValue == value;
                }
                return false;
            };
            bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
                std::string foundValue = extractValue(propsMap);
                if (foundValue != "ZK_NULL"){
                    return foundValue != value;
                }
                return false;
            };
    };

    class SimpleRuleInteger : public SimpleRuleDefault {
        public:
            int value;

            bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
                if(propsMap.count(id)){
                    int foundValue = std::stoi(propsMap[id]);
                    return foundValue == value;
                }
                return false;
            };
            bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
                if(propsMap.count(id)){
                    int foundValue = std::stoi(propsMap[id]);
                    return foundValue != value;
                }
                return false;
            };
    };

    class QueryBuilder{
        public:
            static Query* parseQuery(const char* jsonRule){
                rapidjson::Document doc;
                doc.Parse(jsonRule);
                Query* parsedQuery;
                parsedQuery = new Query();
                parsedQuery->queryType = queryTypeMap[doc["zk_request_type"].GetString()];
                parsedQuery->rule = parse(doc);

                return parsedQuery;
            }

        private:
            static Rule* parse(rapidjson::Document& doc){
                Rule* parsedRule;
                bool isCompositeRule = doc.HasMember("condition");
                if(isCompositeRule){
                    parsedRule = parseCompositeRule(doc);
                }else{
                    parsedRule = parseSimpleRule(doc);
                }

                return parsedRule;
            }

            static Rule* parse(const rapidjson::Value& doc){
                Rule* parsedRule;
                bool isCompositeRule = doc.HasMember("condition");
                if(isCompositeRule){
                    parsedRule = parseCompositeRule(doc);
                }else{
                    parsedRule = parseSimpleRule(doc);
                }

                return parsedRule;
            }

            static Rule* parseCompositeRule(rapidjson::Document& compositeRuleDoc){
                CompositeRule* rule = new CompositeRule();
                std::string conditionString = compositeRuleDoc["condition"].GetString();
                rule->condition = conditionTypeMap[conditionString];
                const rapidjson::Value& rulesDoc = compositeRuleDoc["rules"];
                std::vector<Rule*> vector;
                for (int i = 0; i < rulesDoc.Size(); i++) {
                    Rule* rule = parse(rulesDoc[i]);
                    vector.push_back(rule);
                }
                rule->rules = vector;
                return rule;
            }

            static Rule* parseCompositeRule(const rapidjson::Value& compositeRuleDoc){
                CompositeRule* rule = new CompositeRule();
                rule->condition = conditionTypeMap[compositeRuleDoc["condition"].GetString()];
                const rapidjson::Value& rulesDoc = compositeRuleDoc["rules"];
                std::vector<Rule*> vector;
                for (int i = 0; i < rulesDoc.Size(); i++) {
                    Rule* rule = parse(rulesDoc[i]);
                    vector.push_back(rule);
                }
                rule->rules = vector;
                return rule;
            }

            static Rule* parseWorkloadIdentifierRule(rapidjson::Document& ruleDoc){
                CompositeRule* rule;
                // rule->condition = ConditionType::OR;
                
                // rapidjson::Value value = ruleDoc["value"];
                // std::string sourceOrDestination = ruleDoc["id"].GetString();

                // //TraceRule
                // SimpleRuleString* traceRule;
                // traceRule->id = "trace_role";
                // traceRule->type = FieldType::STRING;
                // traceRule->input = FieldType::STRING;
                // traceRule->value = "server";
                // rule->rules.push_back(traceRule);

                // if(value.HasMember("ip")){
                //     SimpleRuleString* ipRule;
                //     ipRule->id = "remote_addr";
                //     ipRule->type = FieldType::STRING;
                //     ipRule->input = FieldType::STRING;
                //     ipRule->value = "10.0.0.4";
                //     //remote_addr
                //     rule->rules.push_back(ipRule);
                // }

                return rule;
            }

            static Rule* parseWorkloadIdentifierRule(const rapidjson::Value& ruleDoc){
                CompositeRule* rule = new CompositeRule();
                rule->condition = ConditionType::AND;
                
                const rapidjson::Value& value = ruleDoc["value"];
                std::string sourceOrDestination = ruleDoc["id"].GetString();

                //TraceRule
                SimpleRuleString* traceRule = new SimpleRuleString();
                traceRule->id = "trace_role";
                traceRule->type = FieldType::STRING;
                traceRule->input = "string";
                traceRule->value = "server";
                rule->rules.push_back(traceRule);

                //delete once done:
                // for (auto r : rule->rules) {
                //     delete r;
                // }

                //remote_addr
                if(value.HasMember("ip")){
                    SimpleRuleString* ipRule = new SimpleRuleString();
                    ipRule->id = "remote_addr";
                    ipRule->type = FieldType::STRING;
                    ipRule->input = "string";
                    ipRule->value = "10.0.0.4";
                    rule->rules.push_back(ipRule);
                }

                return rule;
            }

            static Rule* parseSimpleRule(rapidjson::Document& ruleDoc){
                SimpleRule* rule;
                FieldType fieldType = fieldTypeMap[ruleDoc["type"].GetString()];
                switch(fieldType){
                    case FieldType::STRING:
                        rule = new SimpleRuleString();
                        ((SimpleRuleString*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case FieldType::INTEGER:
                        rule = new SimpleRuleInteger();
                        ((SimpleRuleInteger*)rule)->value = ruleDoc["value"].GetInt();
                        break;
                    case FieldType::KEY_MAP:
                        rule = new SimpleRuleKeyValue();
                        ((SimpleRuleKeyValue*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case FieldType::WORKLOAD_IDENTIFIER:
                        return parseWorkloadIdentifierRule(ruleDoc);
                        break;

                    default:
                        break;
                }
                // rule->field = ruleDoc["field"].GetString();
                rule->id = ruleDoc["id"].GetString();
                rule->type = fieldType;
                if (ruleDoc.HasMember("key")){
                    rule->key = ruleDoc["key"].GetString();
                }
                
                rule->operatorType = operatorTypeMap[ruleDoc["operator"].GetString()];
                return rule;
            }

            static Rule* parseSimpleRule(const rapidjson::Value& ruleDoc){
                SimpleRule* rule;
                std::string typeString = ruleDoc["type"].GetString();
                FieldType fieldType = fieldTypeMap[typeString];
                switch(fieldType){
                    case FieldType::STRING:
                        rule = new SimpleRuleString();
                        ((SimpleRuleString*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case FieldType::INTEGER:
                        rule = new SimpleRuleInteger();
                        ((SimpleRuleInteger*)rule)->value = ruleDoc["value"].GetInt();
                        break;
                    case FieldType::KEY_MAP:
                        rule = new SimpleRuleKeyValue();
                        ((SimpleRuleKeyValue*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case FieldType::WORKLOAD_IDENTIFIER:
                        return parseWorkloadIdentifierRule(ruleDoc);
                        break;

                    default:
                        break;
                }
                // rule->field = ruleDoc["field"].GetString();
                rule->id = ruleDoc["id"].GetString();
                rule->type = fieldType;
                if (ruleDoc.HasMember("key")){
                    rule->key = ruleDoc["key"].GetString();
                }
                rule->operatorType = operatorTypeMap[ruleDoc["operator"].GetString()];
                return rule;
            }

    };

}