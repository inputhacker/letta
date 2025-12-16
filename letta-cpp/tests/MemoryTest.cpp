#include "Memory.hpp"
#include <gtest/gtest.h>

class MemoryTest : public ::testing::Test {
protected:
    Memory memory;

    void SetUp() override {
        // Base setup, usually blank memory but initialization happens in constructor or init method
        // Memory has explicit init
        memory.initializeDefault();
    }
};

TEST_F(MemoryTest, InitializesDefaultBlocks) {
    auto blocks = memory.getBlocks();
    EXPECT_GE(blocks.size(), 2);
    
    // Check for 'human' and 'persona'
    bool foundHuman = false;
    bool foundPersona = false;
    for (const auto& b : blocks) {
        if (b.label == "human") foundHuman = true;
        if (b.label == "persona") foundPersona = true;
    }
    EXPECT_TRUE(foundHuman);
    EXPECT_TRUE(foundPersona);
}

TEST_F(MemoryTest, AddBlock) {
    MemoryBlock block;
    block.label = "test_block";
    block.value = "test_value";
    block.limit = 100;
    block.read_only = false;
    
    memory.addBlock(block);
    
    auto* retrieved = memory.getBlock("test_block");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->value, "test_value");
}

TEST_F(MemoryTest, RemoveBlock) {
    MemoryBlock block;
    block.label = "to_remove";
    block.value = "val";
    memory.addBlock(block);
    
    ASSERT_NE(memory.getBlock("to_remove"), nullptr);
    
    memory.removeBlock("to_remove");
    EXPECT_EQ(memory.getBlock("to_remove"), nullptr);
}

TEST_F(MemoryTest, UpdateBlock) {
    memory.updateBlock("human", "New Human Value");
    
    auto* human = memory.getBlock("human");
    ASSERT_NE(human, nullptr);
    EXPECT_EQ(human->value, "New Human Value");
}

TEST_F(MemoryTest, CompileContainsBlockValues) {
    std::string compiled = memory.compile();
    EXPECT_NE(compiled.find("Chad"), std::string::npos); // Default human name
    EXPECT_NE(compiled.find("Sam"), std::string::npos); // Default persona name
}
