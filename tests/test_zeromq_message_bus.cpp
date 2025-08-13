#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include "core/message_bus.h"

using namespace swarm;

class ZeroMQMessageBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        messageBus = std::make_unique<MessageBus>();
        messageBus->start();
    }
    
    void TearDown() override {
        if (messageBus) {
            messageBus->stop();
        }
    }
    
    std::unique_ptr<MessageBus> messageBus;
    std::atomic<int> messageCount{0};
    std::string lastTopic;
    std::string lastMessage;
};

TEST_F(ZeroMQMessageBusTest, BasicPublishSubscribe) {
    // Subscribe to a topic
    messageBus->subscribe("test.topic", [this](const std::string& topic, const std::string& message) {
        lastTopic = topic;
        lastMessage = message;
        messageCount++;
    });
    
    // Publish a message
    messageBus->publish("test.topic", "Hello ZeroMQ!");
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(messageCount.load(), 1);
    EXPECT_EQ(lastTopic, "test.topic");
    EXPECT_EQ(lastMessage, "Hello ZeroMQ!");
}

TEST_F(ZeroMQMessageBusTest, AsyncPublishSubscribe) {
    // Subscribe to a topic
    messageBus->subscribe("async.topic", [this](const std::string& topic, const std::string& message) {
        lastTopic = topic;
        lastMessage = message;
        messageCount++;
    });
    
    // Publish async message
    messageBus->publishAsync("async.topic", "Async Hello ZeroMQ!");
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(messageCount.load(), 1);
    EXPECT_EQ(lastTopic, "async.topic");
    EXPECT_EQ(lastMessage, "Async Hello ZeroMQ!");
}

TEST_F(ZeroMQMessageBusTest, MultipleSubscribers) {
    std::atomic<int> subscriber1Count{0};
    std::atomic<int> subscriber2Count{0};
    
    // Subscribe with two different handlers
    messageBus->subscribe("multi.topic", [&subscriber1Count](const std::string& topic, const std::string& message) {
        subscriber1Count++;
    });
    
    messageBus->subscribe("multi.topic", [&subscriber2Count](const std::string& topic, const std::string& message) {
        subscriber2Count++;
    });
    
    // Publish a message
    messageBus->publish("multi.topic", "Multi subscriber test");
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(subscriber1Count.load(), 1);
    EXPECT_EQ(subscriber2Count.load(), 1);
}

TEST_F(ZeroMQMessageBusTest, MessageCount) {
    // Subscribe to a topic
    messageBus->subscribe("count.topic", [](const std::string& topic, const std::string& message) {
        // Do nothing, just count
    });
    
    // Publish multiple messages
    for (int i = 0; i < 5; i++) {
        messageBus->publish("count.topic", "Message " + std::to_string(i));
    }
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(messageBus->getMessageCount(), 5);
}

TEST_F(ZeroMQMessageBusTest, SubscriberCount) {
    // Subscribe to a topic
    messageBus->subscribe("subscriber.count.topic", [](const std::string& topic, const std::string& message) {});
    
    EXPECT_EQ(messageBus->getSubscriberCount("subscriber.count.topic"), 1);
    EXPECT_EQ(messageBus->getSubscriberCount("nonexistent.topic"), 0);
}

TEST_F(ZeroMQMessageBusTest, IsRunning) {
    EXPECT_TRUE(messageBus->isRunning());
    
    messageBus->stop();
    EXPECT_FALSE(messageBus->isRunning());
}

TEST_F(ZeroMQMessageBusTest, ErrorHandling) {
    // Subscribe with a handler that throws an exception
    messageBus->subscribe("error.topic", [](const std::string& topic, const std::string& message) {
        throw std::runtime_error("Test exception");
    });
    
    // Also subscribe with a normal handler
    std::atomic<bool> normalHandlerCalled{false};
    messageBus->subscribe("error.topic", [&normalHandlerCalled](const std::string& topic, const std::string& message) {
        normalHandlerCalled = true;
    });
    
    // Publish a message - should not crash
    EXPECT_NO_THROW({
        messageBus->publish("error.topic", "Error test");
    });
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Normal handler should still be called
    EXPECT_TRUE(normalHandlerCalled.load());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
