#include "log.h"

std::string safeSubstr(const std::string& str, size_t maxLength) {
    size_t len = 0;
    size_t lastValidIndex = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        if ((str[i] & 0xC0) != 0x80) {  // If this byte is the start of a new UTF-8 character
            if (len > maxLength) {
                break;
            }
            lastValidIndex = i;
        }
        ++len;
    }
    return str.substr(0, lastValidIndex);
}
Log::Log(std::string logLevel, std::string threat, int responseCode, std::string message, std::string timestamp, std::string ip){
    this->logLevel = logLevel;
    this->threat = threat;
    this->responseCode = responseCode;
    this->message = message;
    this->timestamp = timestamp;
    this->ip = ip;
}

json Log::toJson(){
    json j;
    j["logLevel"] = this->logLevel;
    j["ip"] = this->ip;
    j["timestamp"] = this->timestamp;
    j["responseCode"] = this->responseCode;
    if(this->message.length() > 100)
        j["message"] = safeSubstr(this->message, 100) + ". . .";
    else
        j["message"] = this->message;
    j["threat"] = this->threat;
    return j;
}

