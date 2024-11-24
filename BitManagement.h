#ifndef BIT_MANAGEMENT_H
#define BIT_MANAGEMENT_H

#include <string>
#include <json/json.h>
#include <sstream> // Include this header for std::istringstream

struct BitTest
{
    std::string name;
    bool pass;
    uint32_t value;

};


class BitManagement {
public:



    BitManagement(const std::string& socketPath, const std::string& configFile);
    Json::Value sendCommand(const std::string& command);
    
    Json::Value readLatestResults();
    Json::Value readPBITResults();
    Json::Value performIBIT();
    Json::Value changeCBITTime(int newTime);
    Json::Value readCBITTime();
    Json::Value performTest(std::string testname);
    void printHandleResponse(const Json::Value& response);
    BitTest handleResponse(const Json::Value& response);
    std::vector<uint8_t> CbitHandleResponse (const Json::Value& response);
    uint64_t convertToBitResults(const Json::Value& response);
   
    
private:
    std::string socketPath_;
    std::string configFile_;
    bool loadConfig(int &min_memory, int &max_used_percent);
    bool translateResultField(std::string result);
    uint64_t bitResult;
};

#endif // BIT_MANAGEMENT_H
