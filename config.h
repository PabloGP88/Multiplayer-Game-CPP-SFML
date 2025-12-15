#pragma once
#include <string>
#include <fstream>
#include <sstream>

class Config {
public:
    static std::string getServerIP() {
        return readValue("SERVER_IP", "127.0.0.1");
    }
    
    static unsigned short getServerPort() {
        const std::string port = readValue("SERVER_PORT", "53000");
        return static_cast<unsigned short>(std::stoi(port));
    }

private:
    static std::string readValue(const std::string& key, const std::string& defaultValue) {
        std::ifstream config("config.txt");
        if (!config.is_open()) return defaultValue;
        
        std::string line;
        while (std::getline(config, line)) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string fileKey = line.substr(0, pos);
                if (fileKey == key) {
                    return line.substr(pos + 1);
                }
            }
        }
        return defaultValue;
    }
};