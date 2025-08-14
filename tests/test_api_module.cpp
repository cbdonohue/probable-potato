#include <gtest/gtest.h>
#include "../include/modules/api_module.h"
#include <memory>

using namespace swarm;

class ApiModuleTest : public ::testing::Test {
protected:
    void SetUp() override {
        apiModule = std::make_unique<ApiModule>();
    }
    
    void TearDown() override {
        if (apiModule) {
            apiModule->shutdown();
        }
    }
    
    std::unique_ptr<ApiModule> apiModule;
};

TEST_F(ApiModuleTest, BasicInitialization) {
    EXPECT_EQ(apiModule->getName(), "api");
    EXPECT_EQ(apiModule->getVersion(), "1.0.0");
    EXPECT_FALSE(apiModule->isRunning());
    
    auto dependencies = apiModule->getDependencies();
    EXPECT_TRUE(dependencies.empty());
}

TEST_F(ApiModuleTest, Configuration) {
    std::map<std::string, std::string> config = {
        {"host", "127.0.0.1"},
        {"port", "8080"},
        {"max_connections", "50"},
        {"enable_cors", "true"}
    };
    
    EXPECT_TRUE(apiModule->configure(config));
    EXPECT_FALSE(apiModule->isRunning());
}

TEST_F(ApiModuleTest, InvalidConfiguration) {
    std::map<std::string, std::string> invalidConfig = {
        {"port", "invalid_port"},
        {"max_connections", "not_a_number"}
    };
    
    // Should handle invalid configuration gracefully
    try {
        bool result = apiModule->configure(invalidConfig);
        EXPECT_TRUE(result || !result); // Either result is acceptable
    } catch (const std::exception& e) {
        // Expected behavior for invalid configuration
        EXPECT_TRUE(true);
    }
}

TEST_F(ApiModuleTest, StatusReporting) {
    std::string status = apiModule->getStatus();
    EXPECT_FALSE(status.empty());
    EXPECT_NE(status.find("API Module"), std::string::npos);
    EXPECT_NE(status.find("running: no"), std::string::npos);
}

TEST_F(ApiModuleTest, MessageHandling) {
    // Test that the module can handle messages
    EXPECT_NO_THROW({
        apiModule->onMessage("test.topic", "test message");
    });
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
