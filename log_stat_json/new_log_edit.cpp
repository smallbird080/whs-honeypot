#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include "json.hpp"

using json = nlohmann::json;

const std::vector<std::string> knownBotsOrScripts = {
    "python-requests", "curl", "wget", "PostmanRuntime", "bot", "crawler", "spider"
};

const size_t minUserAgentLength = 5;
bool isSuspiciousUserAgent(const std::string &userAgent);
bool isKnownBotOrScript(const std::string &userAgent);

const std::regex dirTraversalRegex(R"(\/\.\.\/|\/\.\.$|\/\.\.\/\.\.)");
const std::regex xssRegex(R"(<script>|javascript:)");
const std::regex sqlInjectionRegex(R"((union select|select .* from|insert into|drop table|update .* set))");
const std::regex malformedRequestRegex(R"(PRI|CONNECT|TRACE|TRACK|DEBUG|OPTIONS|PATCH|TRACE|PUT)");

void addSSH(json &attack_IP, json& summary);
void addIPcnt(const json &logEntry, json &attack_IP, bool danger_log);

std::map<std::string, int> classifyLog(const json &logEntry, bool &identified) {
    std::map<std::string, std::vector<std::string>> categories = {
        {"admin_access", {"admin"}},
        {"user_login", {"login"}},
        {"file_access", {".config", ".backup", ".secret", ".env", ".json", ".xml", ".yml", "yaml", ".php", ".bak"}},
        {"port_scan", {"nmap", "masscan"}},
        {"bot_or_script_access", {}},
        {"invalid_or_malformed_requests", {}}
    };

    std::map<std::string, int> counts;
    for (const auto& category : categories) {
        counts[category.first] = 0;
    }

    std::string request = logEntry["request"].get<std::string>();
    std::string userAgent = logEntry["http_user_agent"].get<std::string>();

    identified = false;
    for (const auto& category : categories) {
        for (const auto& keyword : category.second) {
            if (request.find(keyword) != std::string::npos || userAgent.find(keyword) != std::string::npos) {
                counts[category.first]++;
                identified = true;
            }
        }
    }

    if (std::regex_search(request, dirTraversalRegex)) {
        counts["directory_traversal"]++;
        identified = true;
    }
    if (std::regex_search(request, xssRegex) || std::regex_search(userAgent, xssRegex)) {
        counts["xss_attempts"]++;
        identified = true;
    }
    if (std::regex_search(request, sqlInjectionRegex)) {
        counts["sql_injection_attempts"]++;
        identified = true;
    }
    if (std::regex_search(request, malformedRequestRegex)) {
        counts["invalid_or_malformed_requests"]++;
        identified = true;
    }
    if (isKnownBotOrScript(userAgent)) {
        counts["bot_or_script_access"]++;
        identified = true;
    }
    if (isSuspiciousUserAgent(userAgent)) {
        counts["bot_or_script_access"]++;
        identified = true;
    }

    return counts;
}

bool isKnownBotOrScript(const std::string &userAgent) {
    for (const auto& bot : knownBotsOrScripts) {
        if (userAgent.find(bot) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool isSuspiciousUserAgent(const std::string &userAgent) {
    return userAgent.empty() || userAgent.length() < minUserAgentLength || isKnownBotOrScript(userAgent);
}

bool isNormalLog(const json &logEntry, bool identified) {
    int status = std::stoi(logEntry["status"].get<std::string>());
    return status < 400 && !identified && !isSuspiciousUserAgent(logEntry["http_user_agent"].get<std::string>());
}

void removeUTF8EncodedCharacters(std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) { return c >= 128; }), str.end());
}

json analyzeLogs(const std::string &jsonFilePath, json &attack_IP) {
    std::ifstream inputFile(jsonFilePath);
    std::string line;
    json summary = {
        {"total_log_count", 0},
        {"safe_log_count", 0},
        {"danger_log_count", 0},
        {"danger_log", {
            {"admin_access", 0},
            {"user_login", 0},
            {"file_access", 0},
            {"port_scan", 0},
            {"directory_traversal", 0},
            {"xss_attempts", 0},
            {"sql_injection_attempts", 0},
            {"bot_or_script_access", 0},
            {"invalid_or_malformed_requests", 0},
            {"unidentified", 0},
            {"ssh", 0}
        }}
    };

    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }

        removeUTF8EncodedCharacters(line);
        json logEntry = json::parse(line);
        summary["total_log_count"] = summary["total_log_count"].get<int>() + 1;

        bool identified = false;
        std::map<std::string, int> logCounts = classifyLog(logEntry, identified);
        bool isDangerLog = false;
        int dangerLogCategories = 0;

        for (const auto& count : logCounts) {
            if (count.second > 0) {
                summary["danger_log"][count.first] = summary["danger_log"][count.first].get<int>() + count.second;
                // if (count.first != "unidentified") {
                //     isDangerLog = true;
                // }
                dangerLogCategories++;
            }
        }

        if (isNormalLog(logEntry, identified)) {
            summary["safe_log_count"] = summary["safe_log_count"].get<int>() + 1;
        } 
        else if(identified){
            summary["danger_log_count"] = summary["danger_log_count"].get<int>() + 1;
        } 

        int status = std::stoi(logEntry["status"].get<std::string>());
        if (dangerLogCategories == 0 && status >= 400 
        && !isKnownBotOrScript(logEntry["http_user_agent"].get<std::string>())
         && !isSuspiciousUserAgent(logEntry["http_user_agent"].get<std::string>())) {
            // && !isKnownBotOrScript(logEntry["http_user_agent"].get<std::string>()) && !isSuspiciousUserAgent(logEntry["http_user_agent"].get<std::string>())
            summary["danger_log"]["unidentified"] = summary["danger_log"]["unidentified"].get<int>() + 1;
            summary["danger_log_count"] = summary["danger_log_count"].get<int>() + 1;
        }

        addIPcnt(logEntry, attack_IP, identified);
    }

    addSSH(attack_IP, summary);
    inputFile.close();
    return summary;
}

void saveResultsToJson(const json &results, const std::string &outputFilePath) {
    std::ofstream outputFile(outputFilePath);
    outputFile << results.dump(4);
    outputFile.close();
}

void addIPcnt(const json &logEntry, json &attack_IP, bool danger_log) {
    std::string addr = logEntry["remote_addr"].get<std::string>();
    // std::cout<<addr<<std::endl;
    if(danger_log){
        bool found = false;
        for(auto& ip : attack_IP){
            if(ip["IP"] == addr){
                ip["count"] = ip["count"].get<int>() + 1;
                found = true;
                break;
            }
        }
        if(!found){
            json new_ip{
                {"IP", addr},
                {"count", 1}
            };
            attack_IP.push_back(new_ip);
        }
    
    }
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

void addSSH(json &attack_IP, json& summary){
    std::ifstream inputFile("ssh_log.txt");
    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.find("btmp begins") != std::string::npos) {
            return;
        }
        summary["danger_log"]["ssh"] = summary["danger_log"]["ssh"].get<int>() + 1;
        summary["danger_log_count"] = summary["danger_log_count"].get<int>() + 1;
        summary["total_log_count"] = summary["total_log_count"].get<int>() + 1;
        std::string addr = trim(line.substr(line.find("ssh:notty")+12,15));
        bool found = false;
        for(auto& ip : attack_IP){
            if(ip["IP"] == addr){
                ip["count"] = ip["count"].get<int>() + 1;
                found = true;
                break;
            }
        }
        if(!found){
            json new_ip{
                {"IP", addr},
                {"count", 1}
            };
            attack_IP.push_back(new_ip);
        }
    }
}

int main(int argc, char **argv) {
    std::string jsonFilePath = argv[1];
    std::string outputFilePath = "analysis_results.json";
    std::string attack_IP_count = "attack_IP_count.json";

    json attack_IP = json::array();
    json results = analyzeLogs(jsonFilePath, attack_IP);

    saveResultsToJson(results, outputFilePath);

    std::ofstream attack_IP_file(attack_IP_count);
    attack_IP_file << std::setw(4) << attack_IP << std::endl;

    std::cout << "Analysis results saved to " << outputFilePath << std::endl;

    return 0;
}