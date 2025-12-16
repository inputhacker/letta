#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct MemoryBlock {
    std::string label;
    std::string value;
    int limit; // Character limit, roughly
    bool read_only;
    
    json toJson() const;
};

class Memory {
public:
    Memory();
    
    // Initialize with default blocks
    void initializeDefault();

    // Compile memory into the system prompt string
    std::string compile() const;

    // Get a block by label
    MemoryBlock* getBlock(const std::string& label);
    
    // Update a block's value
    bool updateBlock(const std::string& label, const std::string& new_value);

    // List all blocks
    std::vector<MemoryBlock> getBlocks() const;

    // Add a new block
    void addBlock(const MemoryBlock& block);

    // Remove a block by label
    void removeBlock(const std::string& label);

private:
    std::map<std::string, MemoryBlock> blocks;
    std::vector<std::string> block_order; // To maintain consistent compilation order
};
