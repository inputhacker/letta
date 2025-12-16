#pragma once

#include "Memory.hpp"
#include "LLMClient.hpp"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Agent {
public:
    Agent(const std::string& api_key, const std::string& model = "gemini-2.5-flash");
    
    // Main interaction step
    void step(const std::string& user_message);
    
    // Get the current memory state string
    std::string getMemoryDump() const;

    // Add a new memory block
    void addMemoryBlock(const std::string& label, const std::string& value, int limit = 2000, bool read_only = false);

    // Remove a memory block
    void removeMemoryBlock(const std::string& label);

private:
    Memory memory;
    LLMClient llm;
    std::vector<json> messages;
    
    // Tools
    std::vector<json> tools;

    // Helper: Rebuild system prompt
    void rebuildSystemPrompt();
    
    // Helper: Execute a tool call
    json executeTool(const std::string& tool_name, const json& arguments);
};
