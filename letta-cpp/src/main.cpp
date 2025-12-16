#include "Agent.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    const char* api_key_env = std::getenv("GEMINI_API_KEY");
    if (!api_key_env) {
        std::cerr << "Error: GEMINI_API_KEY environment variable not set." << std::endl;
        return 1;
    }
    std::string api_key = api_key_env;

    std::cout << "Initializing Letta C++ Agent..." << std::endl;
    Agent agent(api_key);
    
    std::cout << "Agent Initialized.\n" << std::endl;
    std::cout << "Current Memory:\n" << "----------------\n" << agent.getMemoryDump() << "\n----------------\n" << std::endl;
    std::cout << "Starting chat. Type 'quit' or 'exit' to leave.\n" << std::endl;

    std::string user_input;
    while (true) {
        std::cout << "\033[1;34mYou: \033[0m";
        std::getline(std::cin, user_input);

        if (user_input == "quit" || user_input == "exit") {
            break;
        }

        if (user_input == "/add_scratchpad") {
            agent.addMemoryBlock("scratchpad", "Use this block for temporary thoughts.", 500);
            std::cout << "Added 'scratchpad' block.\n" << std::endl;
            continue;
        }

        if (user_input == "/remove_scratchpad") {
            agent.removeMemoryBlock("scratchpad");
            std::cout << "Removed 'scratchpad' block.\n" << std::endl;
            continue;
        }

        if (user_input.empty()) continue;

        agent.step(user_input);
        
        // Debug dump memory occasionally or on request?
        // std::cout << "[Debug] Memory State:\n" << agent.getMemoryDump() << std::endl;
    }

    return 0;
}
