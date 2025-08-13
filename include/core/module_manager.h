#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include "module.h"
#include "message_bus.h"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>

namespace swarm {

// Module factory function type
using ModuleFactory = std::function<std::unique_ptr<Module>()>;

// Module manager for handling module lifecycle and dependencies
class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();
    
    // Module registration
    void registerModule(const std::string& name, ModuleFactory factory);
    void unregisterModule(const std::string& name);
    
    // Module lifecycle
    bool loadModule(const std::string& name, const std::map<std::string, std::string>& config = {});
    bool unloadModule(const std::string& name);
    bool startModule(const std::string& name);
    bool stopModule(const std::string& name);
    
    // Module management
    void startAllModules();
    void stopAllModules();
    void shutdownAllModules();
    
    // Module access
    Module* getModule(const std::string& name) const;
    std::vector<std::string> getLoadedModules() const;
    std::vector<std::string> getRunningModules() const;
    
    // Dependency management
    bool resolveDependencies(const std::string& moduleName);
    std::vector<std::string> getModuleDependencies(const std::string& moduleName) const;
    
    // Message bus access
    MessageBus* getMessageBus() { return &messageBus_; }
    
    // Status and monitoring
    std::map<std::string, std::string> getModuleStatuses() const;
    bool isModuleRunning(const std::string& name) const;

private:
    struct ModuleInfo {
        std::unique_ptr<Module> module;
        ModuleFactory factory;
        bool loaded;
        bool running;
        std::map<std::string, std::string> config;
    };
    
    bool checkDependencies(const std::vector<std::string>& dependencies) const;
    void loadModuleDependencies(const std::string& moduleName);
    
    std::map<std::string, ModuleInfo> modules_;
    MessageBus messageBus_;
    bool initialized_;
};

} // namespace swarm

#endif // MODULE_MANAGER_H
