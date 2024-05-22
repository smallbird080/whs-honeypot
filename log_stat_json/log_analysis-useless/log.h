#include "json.hpp"

using json = nlohmann::json;

enum LogLevel{
    INFO,
    WARNING,
    ERROR
};


class Log{
    private:
        std::string logLevel;
        std::string threat;
        int responseCode;
        std::string message;
        std::string timestamp;
        std::string ip;
    public:
        Log(std::string logLevel, std::string threat, int responseCode, std::string message, std::string timestamp, std::string ip);
        json toJson();

};