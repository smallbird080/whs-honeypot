#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>

void make_arr() {
    // nlohmann::json 객체 생성
    nlohmann::json j_object;
    nlohmann::json j_array = nlohmann::json::array();
    std::string json_str;
    std::string time,addr,request,method,status,user_agent;
    std::ofstream err_json("err_log.txt");
    // json 파일 읽기
    std::ifstream file("log.json");
    while(std::getline(file, json_str)) {
        try{
            j_object = nlohmann::json::parse(json_str);
        } catch(nlohmann::json::parse_error& e) {
            err_json << json_str << std::endl;
            continue;
        }
        j_array.push_back(j_object);
    }

    // 새로운 json 파일에 json 배열 쓰기
    std::ofstream o("new_log.json");
    o << std::setw(4) << j_array << std::endl;
}

int main() {
    make_arr();
    return 0;
}