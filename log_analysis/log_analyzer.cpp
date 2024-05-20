#include "log.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>

std::map<LogLevel, std::string> LogLevelToString = {
    {INFO, "INFO"},
    {WARNING, "WARNING"},
    {ERROR, "ERROR"}
};

std::vector<std::string> SAFELOGS = {"favicon.ico", "robots.txt", "sitemap.xml", "\" 200 -", "/.well-known/security.txt"};

std::string findThreat(std::string message);

void loginAttempt(int &responseCode, std::string &threat, std::__1::string &line, std::__1::vector<nlohmann::json_abi_v3_11_3::json, std::__1::allocator<nlohmann::json_abi_v3_11_3::json>> &logs, LogLevel logLevel);

void notWebLog(std::__1::string &line, std::__1::ifstream &logFile, int &retFlag);

void updateStats(int* stats, std::string threat, int responseCode);

int main()
{
    std::regex integer_regex("\\d+");

    std::string logFilePath = "app.log";

    std::ifstream logFile(logFilePath);


    int stats[9] = {0,0,0,0,0,0,0,0,0};


    if (!logFile.is_open())
    {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        return 1;
    }

    std::vector<nlohmann::json> logs;
    std::string line;
    while (std::getline(logFile, line))
    {
        int retFlag;
        notWebLog(line, logFile, retFlag);
        if (retFlag == 3)
            continue;
        int responseCode = 0;
        std::string threat = "ETC";
        LogLevel logLevel = line.find("INFO") != std::string::npos ? INFO : line.find("WARNING") != std::string::npos ? WARNING
                                                                                                                      : ERROR;
        if (line.find("INFO:root") != std::string::npos){
            updateStats(stats, threat, responseCode);
            loginAttempt(responseCode, threat, line, logs, logLevel);
            continue;
        }
        std::string message = line.substr(line.find("] ") + 2);
        std::string timestamp = line.substr(line.find("[") + 1, line.find("]") - line.find("[") - 1);
        std::string ip = line.substr(line.find("werkzeug:") + 9, line.find(" - -") - line.find("werkzeug:") - 9);
        if (logLevel == ERROR)
        {   
            if (std::regex_match(line.substr(line.find("code ") + 5, 3), integer_regex))
                responseCode = std::stoi(line.substr(line.find("code ") + 5, 3));
            std::getline(logFile, line);
            message = line.substr(line.find("] ") + 2);
        }
        else
        {   
            if (std::regex_match(line.substr(line.find("\" ") + 2, 3), integer_regex))
            responseCode = std::stoi(line.substr(line.find("\" ") + 2, 3));
        }
        threat = findThreat(message);
        updateStats(stats, threat, responseCode);
        if(threat != "SAFE")
            logs.push_back(Log(LogLevelToString[logLevel], threat, responseCode, message, timestamp, ip).toJson());
    }
    nlohmann::json logData = logs;
    std::ofstream file("logData.json");
    if (file.is_open())
    {
        file << logData;
        file.close();
    }
    else
    {
        std::cerr << "Unable to open data file";
    }

    json logstats;

    logstats["total_logs"] = stats[0];
    logstats["safe_logs"] = stats[1];
    logstats["admin_attempts"] = stats[2];
    logstats["404"] = stats[3];
    logstats["400"] = stats[4];
    logstats["505"] = stats[5];
    logstats["200"] = stats[6];
    logstats["unknown"] = stats[7];
    logstats["scan"] = stats[8];

    std::ofstream statsFile("logStats.json");
    if (statsFile.is_open())
    {
        statsFile << logstats;
        statsFile.close();
    }
    else
    {
        std::cerr << "Unable to open stats file";
    }
    logFile.close();
}

void notWebLog(std::__1::string &line, std::__1::ifstream &logFile, int &retFlag)
{
    retFlag = 1;
    if (line.find("WARNING: This is a development server") != std::string::npos || line.find("Running on") != std::string::npos || line.find("Press CTRL+C to quit") != std::string::npos)
    {
        {
            retFlag = 3;
            return;
        };
    }
    if (line.find("Exception on") != std::string::npos)
    {
        while (line.find("INFO:werkzeug:") != std::string::npos)
        {
            std::getline(logFile, line);
        }
    }
}

void loginAttempt(int &responseCode, std::string &threat, std::__1::string &line, std::__1::vector<nlohmann::json_abi_v3_11_3::json, std::__1::allocator<nlohmann::json_abi_v3_11_3::json>> &logs, LogLevel logLevel)
{
    responseCode = 200;
    std::string ip = "unknown";
    size_t pos = line.find("IP: ");
    if (pos != std::string::npos)
        ip = line.substr(pos + 4, line.find(",") - pos - 4);
    std::string message = line.substr(line.find("root:") + 5);
    threat = findThreat(message);
    if (threat == "Admin_Access")
        message += " - admin login try";
    else
        message += " - invalid login try";
    logs.push_back(Log(LogLevelToString[logLevel], threat, responseCode, message, "unknown", ip).toJson());
}

std::string findThreat(std::string message)
{   
    if (message.find("admin") != std::string::npos || message.find("Admin") != std::string::npos)
        return "Admin_Access";
    if (message.find("ZGrab") != std::string::npos)
        return "SCAN";
    for (std::string safeLog : SAFELOGS)
    {
        if (message.find(safeLog) != std::string::npos)
            return "SAFE";
    }
    return "ETC";
}

void updateStats(int* stats, std::string threat, int responseCode)
{   
    stats[0]++;
    if (threat == "SAFE")
        stats[1]++;
    else if (threat == "Admin_Access")
        stats[2]++;
    else if (threat == "SCAN")
        stats[8]++;
    else
        stats[7]++;
    if (responseCode == 404)
        stats[3]++;
    else if (responseCode == 400)
        stats[4]++;
    else if (responseCode == 505)
        stats[5]++;
    else if (responseCode == 200)
        stats[6]++;
}



// TODO: total logs, safe logs, unsafe logs num / logs by threat, logs by response code