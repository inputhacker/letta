#include "Memory.hpp"
#include <sstream>
#include <iostream>

json MemoryBlock::toJson() const {
    return {
        {"label", label},
        {"value", value},
        {"limit", limit},
        {"read_only", read_only}
    };
}

Memory::Memory() {}

void Memory::initializeDefault() {
    // Human block
    MemoryBlock human;
    human.label = "human";
    human.value = "Name: Chad\nPersonality: Likes 10x vibe coding.";
    human.limit = 2000;
    human.read_only = false;
    blocks[human.label] = human;
    block_order.push_back("human");

    // Persona block
    MemoryBlock persona;
    persona.label = "persona";
    persona.value = "Name: Sam\nRole: You are a helpful AI assistant called Sam. You are keeping track of facts in your memory.";
    persona.limit = 2000;
    persona.read_only = false;
    blocks[persona.label] = persona;
    block_order.push_back("persona");
}

std::string Memory::compile() const {
    std::stringstream ss;
    ss << "You are a Letta agent. You have access to a set of tools and a memory system.\n";
    ss << "You must use the `send_message` tool to communicate with the user.\n";
    ss << "You must use the `core_memory_append` or `core_memory_replace` tools to update your memory when you learn new facts.\n\n";
    
    ss << "### Memory Blocks\n";
    for (const auto& label : block_order) {
        if (blocks.count(label)) {
            const auto& block = blocks.at(label);
            ss << "Block '" << block.label << "' (" << block.value.length() << "/" << block.limit << " chars):\n";
            ss << block.value << "\n\n";
        }
    }
    
    return ss.str();
}

MemoryBlock* Memory::getBlock(const std::string& label) {
    if (blocks.count(label)) {
        return &blocks[label];
    }
    return nullptr;
}

bool Memory::updateBlock(const std::string& label, const std::string& new_value) {
    if (blocks.count(label)) {
        if (blocks[label].read_only) return false;
        
        // Check limit (soft enforcement for now, just warn)
        if (new_value.length() > blocks[label].limit) {
            std::cerr << "Warning: New value for block '" << label << "' exceeds limit (" 
                      << new_value.length() << " > " << blocks[label].limit << ")" << std::endl;
        }
        
        blocks[label].value = new_value;
        return true;
    }
    return false;
}

std::vector<MemoryBlock> Memory::getBlocks() const {
    std::vector<MemoryBlock> result;
    for (const auto& label : block_order) {
        if (blocks.count(label)) {
            result.push_back(blocks.at(label));
        }
    }
    return result;
}

void Memory::addBlock(const MemoryBlock& block) {
    if (blocks.count(block.label)) {
        std::cerr << "Warning: Block '" << block.label << "' already exists. Overwriting." << std::endl;
        // If overwriting, remove from order first to re-add at end, or just keep position?
        // Let's keep position if exists, otherwise push back.
    } else {
        block_order.push_back(block.label);
    }
    blocks[block.label] = block;
}

void Memory::removeBlock(const std::string& label) {
    if (blocks.count(label)) {
        blocks.erase(label);
        // Remove from order vector
        for (auto it = block_order.begin(); it != block_order.end(); ) {
            if (*it == label) {
                it = block_order.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        std::cerr << "Warning: Block '" << label << "' not found, cannot remove." << std::endl;
    }
}
