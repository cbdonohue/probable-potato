#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>

// Include SwarmApp core components
#include "core/module.h"
#include "core/message_bus.h"
#include "core/module_manager.h"

using namespace swarm;

// Test fixture for SwarmApp core functionality
class SwarmAppCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }
    
    void TearDown() override {
        // Clean up after each test
    }
};

// Test MessageBus basic functionality
TEST_F(SwarmAppCoreTest, MessageBusBasicFunctionality) {
    MessageBus messageBus;
    messageBus.start();
    
    std::atomic<int> messageCount{0};
    std::string receivedTopic;
    std::string receivedMessage;
    
    // Subscribe to a test topic
    messageBus.subscribe("test.topic", [&](const std::string& topic, const std::string& message) {
        receivedTopic = topic;
        receivedMessage = message;
        messageCount++;
    });
    
    // Publish a message
    messageBus.publish("test.topic", "Hello SwarmApp!");
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(messageCount.load(), 1);
    EXPECT_EQ(receivedTopic, "test.topic");
    EXPECT_EQ(receivedMessage, "Hello SwarmApp!");
    
    messageBus.stop();
}

// Test MessageBus async functionality
TEST_F(SwarmAppCoreTest, MessageBusAsyncFunctionality) {
    MessageBus messageBus;
    messageBus.start();
    
    std::atomic<int> messageCount{0};
    
    // Subscribe to a test topic
    messageBus.subscribe("async.topic", [&](const std::string& topic, const std::string& message) {
        messageCount++;
    });
    
    // Publish async message
    messageBus.publishAsync("async.topic", "Async Hello!");
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(messageCount.load(), 1);
    
    messageBus.stop();
}

// Test MessageBus multiple subscribers
TEST_F(SwarmAppCoreTest, MessageBusMultipleSubscribers) {
    MessageBus messageBus;
    messageBus.start();
    
    std::atomic<int> subscriber1Count{0};
    std::atomic<int> subscriber2Count{0};
    
    // Subscribe with two different handlers
    messageBus.subscribe("multi.topic", [&](const std::string& topic, const std::string& message) {
        subscriber1Count++;
    });
    
    messageBus.subscribe("multi.topic", [&](const std::string& topic, const std::string& message) {
        subscriber2Count++;
    });
    
    // Publish a message
    messageBus.publish("multi.topic", "Multi subscriber test");
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(subscriber1Count.load(), 1);
    EXPECT_EQ(subscriber2Count.load(), 1);
    
    messageBus.stop();
}

// Test MessageBus statistics
TEST_F(SwarmAppCoreTest, MessageBusStatistics) {
    MessageBus messageBus;
    messageBus.start();
    
    // Subscribe to a topic
    messageBus.subscribe("stats.topic", [](const std::string& topic, const std::string& message) {
        // Do nothing, just count
    });
    
    // Publish multiple messages
    for (int i = 0; i < 5; i++) {
        messageBus.publish("stats.topic", "Message " + std::to_string(i));
    }
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(messageBus.getMessageCount(), 5);
    EXPECT_EQ(messageBus.getSubscriberCount("stats.topic"), 1);
    EXPECT_EQ(messageBus.getSubscriberCount("nonexistent.topic"), 0);
    
    messageBus.stop();
}

// Test MessageBus lifecycle
TEST_F(SwarmAppCoreTest, MessageBusLifecycle) {
    MessageBus messageBus;
    
    // Initially not running
    EXPECT_FALSE(messageBus.isRunning());
    
    // Start the message bus
    messageBus.start();
    EXPECT_TRUE(messageBus.isRunning());
    
    // Stop the message bus
    messageBus.stop();
    EXPECT_FALSE(messageBus.isRunning());
}

// Test MessageBus error handling
TEST_F(SwarmAppCoreTest, MessageBusErrorHandling) {
    MessageBus messageBus;
    messageBus.start();
    
    // Subscribe with a handler that throws an exception
    messageBus.subscribe("error.topic", [](const std::string& topic, const std::string& message) {
        throw std::runtime_error("Test exception");
    });
    
    // Also subscribe with a normal handler
    std::atomic<bool> normalHandlerCalled{false};
    messageBus.subscribe("error.topic", [&](const std::string& topic, const std::string& message) {
        normalHandlerCalled = true;
    });
    
    // Publish a message - should not crash
    EXPECT_NO_THROW({
        messageBus.publish("error.topic", "Error test");
    });
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Normal handler should still be called
    EXPECT_TRUE(normalHandlerCalled.load());
    
    messageBus.stop();
}

// Test Module base class functionality
TEST_F(SwarmAppCoreTest, ModuleBaseClass) {
    // Create a simple test module
    class TestModule : public Module {
    public:
        TestModule() : running_(false) {}
        
        bool initialize() override { return true; }
        void start() override { running_ = true; }
        void stop() override { running_ = false; }
        void shutdown() override {}
        
        std::string getName() const override { return "test-module"; }
        std::string getVersion() const override { return "1.0.0"; }
        std::vector<std::string> getDependencies() const override { return {}; }
        
        bool isRunning() const override { return running_; }
        std::string getStatus() const override { return running_ ? "running" : "stopped"; }
        
        bool configure(const std::map<std::string, std::string>& config) override { return true; }
        void onMessage(const std::string& topic, const std::string& message) override {}
        
    private:
        bool running_;
    };
    
    TestModule module;
    
    EXPECT_EQ(module.getName(), "test-module");
    EXPECT_EQ(module.getVersion(), "1.0.0");
    EXPECT_FALSE(module.isRunning());
    EXPECT_EQ(module.getStatus(), "stopped");
    
    EXPECT_TRUE(module.initialize());
    module.start();
    EXPECT_TRUE(module.isRunning());
    EXPECT_EQ(module.getStatus(), "running");
    
    module.stop();
    EXPECT_FALSE(module.isRunning());
    EXPECT_EQ(module.getStatus(), "stopped");
}

// Test ModuleManager basic functionality
TEST_F(SwarmAppCoreTest, ModuleManagerBasicFunctionality) {
    ModuleManager manager;
    
    // Test that message bus is available
    EXPECT_NE(manager.getMessageBus(), nullptr);
    EXPECT_TRUE(manager.getMessageBus()->isRunning());
    
    // Test module registration
    manager.registerModule("test-module", []() -> std::unique_ptr<Module> {
        class TestModule : public Module {
        public:
            bool initialize() override { return true; }
            void start() override {}
            void stop() override {}
            void shutdown() override {}
            std::string getName() const override { return "test-module"; }
            std::string getVersion() const override { return "1.0.0"; }
            std::vector<std::string> getDependencies() const override { return {}; }
            bool isRunning() const override { return false; }
            std::string getStatus() const override { return "stopped"; }
            bool configure(const std::map<std::string, std::string>& config) override { return true; }
            void onMessage(const std::string& topic, const std::string& message) override {}
        };
        return std::make_unique<TestModule>();
    });
    
    // Test module loading
    EXPECT_TRUE(manager.loadModule("test-module"));
    EXPECT_TRUE(manager.isModuleRunning("test-module") == false); // Not started yet
    
    // Test module starting
    EXPECT_TRUE(manager.startModule("test-module"));
    EXPECT_TRUE(manager.isModuleRunning("test-module"));
    
    // Test module stopping
    EXPECT_TRUE(manager.stopModule("test-module"));
    EXPECT_FALSE(manager.isModuleRunning("test-module"));
}

// Test ZeroMQ integration
TEST_F(SwarmAppCoreTest, ZeroMQIntegration) {
    MessageBus messageBus;
    messageBus.start();
    
    std::atomic<int> messageCount{0};
    std::string lastMessage;
    
    // Subscribe to ZeroMQ messages
    messageBus.subscribe("zeromq.test", [&](const std::string& topic, const std::string& message) {
        lastMessage = message;
        messageCount++;
    });
    
    // Test synchronous publishing via ZeroMQ
    messageBus.publish("zeromq.test", "ZeroMQ sync test");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(messageCount.load(), 1);
    EXPECT_EQ(lastMessage, "ZeroMQ sync test");
    
    // Test asynchronous publishing via ZeroMQ
    messageBus.publishAsync("zeromq.test", "ZeroMQ async test");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(messageCount.load(), 2);
    EXPECT_EQ(lastMessage, "ZeroMQ async test");
    
    messageBus.stop();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
