#include "BitManagement.h"
#include "Logger.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

#define CONFIG_FILE "/etc/config.json"

BitManagement::BitManagement(const std::string& socketPath, const std::string& configFile)
    : socketPath_(socketPath), configFile_(configFile) {}

Json::Value BitManagement::sendCommand(const std::string &command) {
    int client_fd;
    struct sockaddr_un addr;
    char buf[1024];
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        Logger::log("Socket error", Logger::Level::ERROR);
        return root;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::log("Connect error", Logger::Level::ERROR);
        close(client_fd);
        return root;
    }

    write(client_fd, command.c_str(), command.size());
    int n = read(client_fd, buf, sizeof(buf));
    if (n > 0) {
        buf[n] = '\0';
        std::istringstream s(buf);
        if (!Json::parseFromStream(reader, s, &root, &errs)) {
            //Logger::log("Failed to parse response: " + errs, Logger::Level::ERROR);
        }
    }
    close(client_fd);
    return root;
}

Json::Value BitManagement::readLatestResults() {
    Json::Value root;
    root["command"] = "READ_LATEST_RESULTS";
    std::string command = Json::writeString(Json::StreamWriterBuilder(), root);
   Logger::log("Sending: " + command, Logger::Level::TRACE);
    return sendCommand(command);
}

Json::Value BitManagement::readPBITResults() {
    Json::Value root;
    root["command"] = "READ_PBIT";
    std::string command = Json::writeString(Json::StreamWriterBuilder(), root);
   Logger::log("Sending: " + command, Logger::Level::TRACE);
    return sendCommand(command);
}

Json::Value BitManagement::performIBIT() {
    Json::Value root;
    root["command"] = "PERFORM_IBIT";
    std::string command = Json::writeString(Json::StreamWriterBuilder(), root);
    Logger::log("Sending: " + command, Logger::Level::TRACE);
    return sendCommand(command);
}

Json::Value BitManagement::changeCBITTime(int newTime) {
    Json::Value root;
    root["command"] = "CHANGE_CBIT_TIME";
    root["new_time"] = newTime;
    std::string command = Json::writeString(Json::StreamWriterBuilder(), root);
    Logger::log("Sending: " + command, Logger::Level::TRACE);
    return sendCommand(command);
}

Json::Value BitManagement::readCBITTime() {
    Json::Value root;
    root["command"] = "READ_CBIT_TIME";
    std::string command = Json::writeString(Json::StreamWriterBuilder(), root);
    Logger::log("Sending: " + command, Logger::Level::TRACE);
    return sendCommand(command);
}

Json::Value BitManagement::performTest(std::string testname) {
    Json::Value root;
    root["command"] = "PERFORM_TEST_" + testname;
    std::string command = Json::writeString(Json::StreamWriterBuilder(), root);
    Logger::log("Sending: " + command, Logger::Level::TRACE);
    return sendCommand(command);
}

void BitManagement::printHandleResponse(const Json::Value& response) {
    const Json::Value results = response["results"];
    for (const auto& result : results) {
        std::string test = result["test"].asString();
        std::string result_str = result["result"].asString();
        std::string value = result["value"].asString();
        Logger::log("Test: " + test + ", Result: " + result_str +", Value: " + value, Logger::Level::TRACE);
    }
}

BitTest BitManagement::handleResponse(const Json::Value& response) {
    const Json::Value results = response["results"];
    BitTest test;
    for (const auto& result : results) {
        test.name = result["test"].asString();
        test.pass = translateResultField(result["result"].asString());
        test.value = result["value"].asUInt();
        Logger::log(Logger::Level::TRACE,"Test: %s , Value: %d" ,test.name.c_str(),test.value);
    }
    return test;
}

std::vector<uint8_t> BitManagement::CbitHandleResponse(const Json::Value& response) {
    const Json::Value results = response["results"];
    BitTest test;
    std::vector<uint8_t> data;
    uint16_t uint16Value = 0;
    for (const auto& result : results) {
        test.name = result["test"].asString();
        test.pass = translateResultField(result["result"].asString());
        
       if(test.name == "Temperature_ACPI" || test.name == "Temperature_ISA" || test.name == "Temperature_Core_0"|| test.name == "Temperature_Core_1"|| test.name == "Temperature_Core_2"|| test.name == "Temperature_Core_3")
        test.value = static_cast<uint32_t>((result["value"].asFloat()+50)*100);
       else if(test.name == "Temperature_ACPI" || test.name == "P5V" || test.name == "P12V"|| test.name == "P3V_Battery")
        test.value = static_cast<uint32_t>(result["value"].asFloat()*100);
       else
        test.value = result["value"].asUInt();
        
        //Logger::log(Logger::Level::DEBUG,"Test: %s , Value: %d" ,test.name.c_str(),test.value);
        
         // Convert the uint32_t value to uint16_t
        uint16Value = static_cast<uint16_t>(test.value);
        Logger::log(Logger::Level::TRACE,"Test: %s , ValueString: %s ,ValueSend: %04x" ,test.name.c_str(),result["value"].asString().c_str(),uint16Value);

        // Convert uint16_t to 2 uint8_t and append to data
        data.push_back(static_cast<uint8_t>(uint16Value & 0xFF));        // Least significant byte
        data.push_back(static_cast<uint8_t>((uint16Value >> 8) & 0xFF)); // Most significant byte


    }
    return data;
}

uint64_t BitManagement::convertToBitResults(const Json::Value& response) {
    const Json::Value results = response["results"];
    int index = 7;
    int offsetbytes = 0;
    bitResult = 0;
    for (const auto& result : results) { 
        if (translateResultField(result["result"].asString()))
        {
           bitResult |= (1 << (index+offsetbytes)); 
        }
       
        Logger::log("Test: " + result["test"].asString() + ", Result: " + result["result"].asString() +", Value: " + result["value"].asString(), Logger::Level::TRACE);
        
        if(index == 0)
        {
            index = 7;
            offsetbytes += 8;
        }
        else
        {
            index --;
        }
    }
    return bitResult;
}

bool BitManagement::loadConfig(int &min_memory, int &max_used_percent) {
    // Placeholder for LoadConfig implementation
    // You should implement your configuration loading logic here.
    // Return true if successful, false otherwise.
    return true;
}

bool BitManagement::translateResultField(std::string result)
{
    if (result == "success"){
        return true;
    }
    return false;
}