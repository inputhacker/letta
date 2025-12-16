#include "Agent.hpp"
#include <gtest/gtest.h>

class AgentTest : public ::testing::Test {
protected:
    // Agent requires API key, pass dummy
    Agent agent{"dummy_key"};
};

TEST_F(AgentTest, InitializesWithDefaults) {
    std::string dump = agent.getMemoryDump();
    EXPECT_NE(dump.find("human"), std::string::npos);
    EXPECT_NE(dump.find("persona"), std::string::npos);
}

TEST_F(AgentTest, AddMemoryBlock) {
    agent.addMemoryBlock("agent_test_block", "some value");
    std::string dump = agent.getMemoryDump();
    EXPECT_NE(dump.find("agent_test_block"), std::string::npos);
    EXPECT_NE(dump.find("some value"), std::string::npos);
}

TEST_F(AgentTest, RemoveMemoryBlock) {
    agent.addMemoryBlock("temp_block", "temp value");
    
    // Verify added
    std::string dump1 = agent.getMemoryDump();
    EXPECT_NE(dump1.find("temp_block"), std::string::npos);
    
    agent.removeMemoryBlock("temp_block");
    
    // Verify removed
    std::string dump2 = agent.getMemoryDump();
    EXPECT_EQ(dump2.find("temp_block"), std::string::npos);
}
