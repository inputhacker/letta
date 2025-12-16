#include "Agent.hpp"
#include "Tools.hpp"
#include <iostream>

using json = nlohmann::json;

Agent::Agent(const std::string& api_key, const std::string& model) 
    : llm(api_key, "https://generativelanguage.googleapis.com/v1beta", model) 
{
    memory.initializeDefault();
    tools = Tools::get_all_tools();
    rebuildSystemPrompt();
}

void Agent::rebuildSystemPrompt() {
    std::string system_prompt = memory.compile();
    
    // If messages is empty, add system prompt. 
    // If not empty, update the first message (assuming it's system)
    json system_msg = {
        {"role", "system"},
        {"content", system_prompt}
    };

    if (messages.empty()) {
        messages.push_back(system_msg);
    } else {
        if (messages[0]["role"] == "system") {
            messages[0] = system_msg;
        } else {
            // Should not happen if logic is correct, but safe insert
            messages.insert(messages.begin(), system_msg);
        }
    }
}

std::string Agent::getMemoryDump() const {
    return memory.compile();
}

json Agent::executeTool(const std::string& tool_name, const json& arguments) {
    std::cout << "[Agent] Executing tool: " << tool_name << std::endl;
    // std::cout << "[Agent] Args: " << arguments.dump() << std::endl;

    if (tool_name == "send_message") {
        std::string msg = arguments.value("message", "");
        std::cout << "\n\033[1;32m[Agent (Sam)]: " << msg << "\033[0m\n" << std::endl;
        return {
            {"status", "OK"},
            {"message", "Message sent to user."}
        };
    }
    
    if (tool_name == "core_memory_append") {
        std::string label = arguments.value("label", "");
        std::string content = arguments.value("content", "");
        
        MemoryBlock* block = memory.getBlock(label);
        if (!block) {
            return {{"status", "ERROR"}, {"message", "Block not found: " + label}};
        }
        
        std::string new_value = block->value + "\n" + content;
        if (memory.updateBlock(label, new_value)) {
             // System prompt needs update next turn
             rebuildSystemPrompt();
             return {{"status", "OK"}, {"message", "Memory block '" + label + "' updated."}};
        } else {
             return {{"status", "ERROR"}, {"message", "Failed to update block (maybe read-only?)"}};
        }
    }

    if (tool_name == "core_memory_replace") {
        std::string label = arguments.value("label", "");
        std::string old_content = arguments.value("old_content", "");
        std::string new_content = arguments.value("new_content", "");

        MemoryBlock* block = memory.getBlock(label);
        if (!block) {
            return {{"status", "ERROR"}, {"message", "Block not found: " + label}};
        }
        
        // Simple replace for now, ignoring exact match check of 'old_content' for MVP simplicity
        // But in real Letta, we check if old_content exists.
        
        std::string current_val = block->value;
        size_t pos = current_val.find(old_content);
        if (pos != std::string::npos) {
             current_val.replace(pos, old_content.length(), new_content);
             if (memory.updateBlock(label, current_val)) {
                 rebuildSystemPrompt();
                 return {{"status", "OK"}, {"message", "Memory block '" + label + "' updated."}};
             }
        } else {
             return {{"status", "ERROR"}, {"message", "Old content not found in block."}};
        }
        return {{"status", "ERROR"}, {"message", "Failed to update block."}};
    }

    return {{"status", "ERROR"}, {"message", "Unknown tool: " + tool_name}};
}


void Agent::step(const std::string& user_message) {
    // 1. Add user message
    messages.push_back({
        {"role", "user"},
        {"content", user_message}
    });

    // Loop for tool calls
    int steps = 0;
    const int MAX_STEPS = 5; // Safety limit
    
    while (steps < MAX_STEPS) {
        steps++;
        
        // 2. Call LLM
        json response = llm.chatCompletion(messages, tools);
        
        if (response.contains("error")) {
            std::cerr << "LLM Error: " << response["error"] << std::endl;
            break;
        }

        json choice = response["choices"][0];
        json message = choice["message"];
        
        // Append assistant message to history
        messages.push_back(message);

        // Check for tool calls
        if (message.contains("tool_calls") && !message["tool_calls"].empty()) {
            json tool_calls = message["tool_calls"];
            
            for (const auto& tool_call : tool_calls) {
                std::string id = tool_call["id"];
                std::string name = tool_call["function"]["name"];
                std::string args_str = tool_call["function"]["arguments"];
                json args = json::parse(args_str);

                // Execute
                json tool_result = executeTool(name, args);
                
                // Add tool result to messages
                messages.push_back({
                    {"role", "tool"},
                    {"tool_call_id", id},
                    {"name", name},
                    {"content", tool_result.dump()}
                });

                // If send_message was called, we generally stop the loop and wait for user input, 
                // UNLESS we want to support multiple tool calls in a row.
                // For this MVP, if send_message is called, we consider it a 'yield' point.
                if (name == "send_message") {
                    return; 
                }
            }
        } else {
            // No tool calls (shouldn't happen with Letta usually, as it forces tool calls, but handling it as generic chat)
            std::string content = message.value("content", "");
            if (!content.empty()) {
                std::cout << "[Agent (Raw)]: " << content << std::endl;
            }
            break; 
        }
    }
}

void Agent::addMemoryBlock(const std::string& label, const std::string& value, int limit, bool read_only) {
    MemoryBlock block;
    block.label = label;
    block.value = value;
    block.limit = limit;
    block.read_only = read_only;
    
    memory.addBlock(block);
    rebuildSystemPrompt();
    std::cout << "[Agent] Added memory block '" << label << "'" << std::endl;
}

void Agent::removeMemoryBlock(const std::string& label) {
    memory.removeBlock(label);
    rebuildSystemPrompt();
    std::cout << "[Agent] Removed memory block '" << label << "'" << std::endl;
}
