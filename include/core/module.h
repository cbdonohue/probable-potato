#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>

namespace swarm {

// Forward declarations
class ModuleManager;
class MessageBus;

// Base module interface
class Module {
public:
    virtual ~Module() = default;
    
    // Module lifecycle
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void shutdown() = 0;
    
    // Module identification
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::vector<std::string> getDependencies() const = 0;
    
    // Module status
    virtual bool isRunning() const = 0;
    virtual std::string getStatus() const = 0;
    
    // Configuration
    virtual bool configure(const std::map<std::string, std::string>& config) = 0;
    
    // Message handling
    virtual void onMessage(const std::string& topic, const std::string& message) = 0;
    
    // Module manager access
    void setModuleManager(ModuleManager* manager) { moduleManager_ = manager; }
    ModuleManager* getModuleManager() const { return moduleManager_; }
    
    // Message bus access
    void setMessageBus(MessageBus* bus) { messageBus_ = bus; }
    MessageBus* getMessageBus() const { return messageBus_; }

protected:
    ModuleManager* moduleManager_ = nullptr;
    MessageBus* messageBus_ = nullptr;
    bool running_ = false;
};

} // namespace swarm

#endif // MODULE_H
