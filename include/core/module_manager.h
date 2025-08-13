/**
 * @file module_manager.h
 * @brief Module manager for handling module lifecycle and dependencies
 * @author SwarmApp Development Team
 * @version 1.0.0
 */

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

/**
 * @brief Module factory function type
 * 
 * Factory functions create and return new module instances
 */
using ModuleFactory = std::function<std::unique_ptr<Module>()>;

/**
 * @brief Module manager for handling module lifecycle and dependencies
 * 
 * The ModuleManager class is responsible for managing the lifecycle of all modules
 * in the SwarmApp framework. It handles module registration, loading, starting,
 * stopping, and dependency resolution.
 * 
 * Features:
 * - Module registration and factory management
 * - Dependency resolution and management
 * - Module lifecycle control (load, start, stop, unload)
 * - Status monitoring and reporting
 * - Message bus integration
 * 
 * @note This class is not thread-safe and should be used from a single thread
 * @see Module
 * @see MessageBus
 */
class ModuleManager {
public:
    /**
     * @brief Constructor
     * 
     * Initializes the module manager and message bus
     */
    ModuleManager();
    
    /**
     * @brief Destructor
     * 
     * Shuts down all modules and cleans up resources
     */
    ~ModuleManager();
    
    /**
     * @name Module Registration Methods
     * @{
     */
    
    /**
     * @brief Register a module factory
     * 
     * Registers a factory function that can create instances of a module.
     * 
     * @param name The unique name of the module
     * @param factory The factory function to create module instances
     */
    void registerModule(const std::string& name, ModuleFactory factory);
    
    /**
     * @brief Unregister a module factory
     * 
     * Removes a module factory from the registry.
     * 
     * @param name The name of the module to unregister
     */
    void unregisterModule(const std::string& name);
    
    /** @} */
    
    /**
     * @name Module Lifecycle Methods
     * @{
     */
    
    /**
     * @brief Load a module
     * 
     * Creates a module instance and initializes it with the given configuration.
     * 
     * @param name The name of the module to load
     * @param config Configuration parameters for the module
     * @return true if the module was loaded successfully, false otherwise
     */
    bool loadModule(const std::string& name, const std::map<std::string, std::string>& config = {});
    
    /**
     * @brief Unload a module
     * 
     * Stops and destroys a module instance.
     * 
     * @param name The name of the module to unload
     * @return true if the module was unloaded successfully, false otherwise
     */
    bool unloadModule(const std::string& name);
    
    /**
     * @brief Start a module
     * 
     * Starts a loaded module.
     * 
     * @param name The name of the module to start
     * @return true if the module was started successfully, false otherwise
     */
    bool startModule(const std::string& name);
    
    /**
     * @brief Stop a module
     * 
     * Stops a running module.
     * 
     * @param name The name of the module to stop
     * @return true if the module was stopped successfully, false otherwise
     */
    bool stopModule(const std::string& name);
    
    /** @} */
    
    /**
     * @name Module Management Methods
     * @{
     */
    
    /**
     * @brief Start all loaded modules
     * 
     * Starts all modules that are currently loaded but not running.
     */
    void startAllModules();
    
    /**
     * @brief Stop all running modules
     * 
     * Stops all modules that are currently running.
     */
    void stopAllModules();
    
    /**
     * @brief Shutdown all modules
     * 
     * Stops and unloads all modules, performing a complete shutdown.
     */
    void shutdownAllModules();
    
    /** @} */
    
    /**
     * @name Module Access Methods
     * @{
     */
    
    /**
     * @brief Get a module by name
     * 
     * @param name The name of the module
     * @return Pointer to the module, or nullptr if not found
     */
    Module* getModule(const std::string& name) const;
    
    /**
     * @brief Get list of loaded modules
     * 
     * @return Vector of module names that are currently loaded
     */
    std::vector<std::string> getLoadedModules() const;
    
    /**
     * @brief Get list of running modules
     * 
     * @return Vector of module names that are currently running
     */
    std::vector<std::string> getRunningModules() const;
    
    /** @} */
    
    /**
     * @name Dependency Management Methods
     * @{
     */
    
    /**
     * @brief Resolve module dependencies
     * 
     * Ensures that all dependencies for a module are loaded and running.
     * 
     * @param moduleName The name of the module
     * @return true if dependencies were resolved successfully, false otherwise
     */
    bool resolveDependencies(const std::string& moduleName);
    
    /**
     * @brief Get module dependencies
     * 
     * @param moduleName The name of the module
     * @return Vector of module names that the module depends on
     */
    std::vector<std::string> getModuleDependencies(const std::string& moduleName) const;
    
    /** @} */
    
    /**
     * @name Message Bus Access
     * @{
     */
    
    /**
     * @brief Get the message bus
     * 
     * @return Pointer to the message bus used by the module manager
     */
    MessageBus* getMessageBus() { return &messageBus_; }
    
    /** @} */
    
    /**
     * @name Status and Monitoring Methods
     * @{
     */
    
    /**
     * @brief Get status of all modules
     * 
     * @return Map of module names to their status strings
     */
    std::map<std::string, std::string> getModuleStatuses() const;
    
    /**
     * @brief Check if a module is running
     * 
     * @param name The name of the module
     * @return true if the module is running, false otherwise
     */
    bool isModuleRunning(const std::string& name) const;
    
    /** @} */

private:
    /**
     * @brief Internal module information structure
     * 
     * Contains all information about a registered module
     */
    struct ModuleInfo {
        std::unique_ptr<Module> module;                    ///< The module instance
        ModuleFactory factory;                             ///< Factory function to create the module
        bool loaded;                                       ///< Whether the module is loaded
        bool running;                                      ///< Whether the module is running
        std::map<std::string, std::string> config;         ///< Module configuration
    };
    
    /**
     * @brief Check if all dependencies are satisfied
     * 
     * @param dependencies List of required module names
     * @return true if all dependencies are loaded and running
     */
    bool checkDependencies(const std::vector<std::string>& dependencies) const;
    
    /**
     * @brief Load dependencies for a module
     * 
     * @param moduleName The name of the module whose dependencies to load
     */
    void loadModuleDependencies(const std::string& moduleName);
    
    std::map<std::string, ModuleInfo> modules_;           ///< Registry of all modules
    MessageBus messageBus_;                               ///< Message bus for inter-module communication
    bool initialized_;                                     ///< Whether the manager has been initialized
};

} // namespace swarm

#endif // MODULE_MANAGER_H
