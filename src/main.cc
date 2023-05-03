#include <iostream>
#include "rapidjson/document.h"
#include "zerok/query/query.h"
int main()
{
    std::cout << "\n--------------------\n";
    std::cout << "Starting the main.cc\n";
    //string
    // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":\"HTTP\",\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"}]}";
    //string + int
    // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":\"HTTP\",\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"int_field\",\"field\":\"int_field\",\"type\":\"integer\",\"input\":\"string\",\"operator\":\"equal\",\"value\":35}]}";
    //string + int + keyvalue
    // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":\"HTTP\",\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"int_field\",\"field\":\"int_field\",\"type\":\"integer\",\"input\":\"string\",\"operator\":\"equal\",\"value\":35},{\"id\":\"key_value_field\",\"field\":\"key_value_field\",\"key\":\"/value/value2/value3\",\"type\":\"key-map\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"}]}";
    //string + int + keyvalue + workload
    const char* json = "{\"condition\":\"AND\",\"zk_request_type\":\"HTTP\",\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"int_field\",\"field\":\"int_field\",\"type\":\"integer\",\"input\":\"string\",\"operator\":\"equal\",\"value\":35},{\"id\":\"key_value_field\",\"field\":\"key_value_field\",\"key\":\"/value/value2/value3\",\"type\":\"key-map\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"source\",\"field\":\"source\",\"type\":\"workload-identifier\",\"operator\":\"in\",\"value\":{\"service_name\":\"demo/sofa, demo2/invent\",\"ip\":\"10.43.3.4\",\"pod_name\":\"abc,zxy\"}}]}";
    
    zk::Query* query = zk::QueryBuilder::parseQuery(json);
    std::map<std::string, std::string> propsMap = {
        {"zk_req_type", "HTTP"},
        {"int_field", "35"},
        {"trace_role", "server"},
        {"remote_addr", "10.0.0.4"},
        {"key_value_field", "{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value2\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value3\":\"HTTP\"}}}"},
    };
    std::cout << "query->rule->evaluate(propsMap) " << query->rule->evaluate(propsMap);

    return 0;
}