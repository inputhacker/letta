#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class LLMClient {
public:
    LLMClient(const std::string& api_key, const std::string& base_url = "https://api.openai.com/v1", const std::string& model = "gpt-4");

    json chatCompletion(const std::vector<json>& messages, const std::vector<json>& tools = {});

private:
    std::string api_key;
    std::string base_url;
    std::string model;
    
    // Helper to print debug info
    void printDebug(const std::string& label, const std::string& content);
};
