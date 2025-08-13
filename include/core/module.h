/**
 * @file module.h
 * @brief Base module interface for the SwarmApp framework
 * @author SwarmApp Development Team
 * @version 1.0.0
 */

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

/**
 * @brief Base module interface for the SwarmApp framework
 * 
 * The Module class provides the base interface that all application modules must implement.
 * It defines the lifecycle methods, configuration interface, and message handling capabilities
 * that are common to all modules in the system.
 * 
 * @note All modules should inherit from this class and implement the pure virtual methods.
 * @see ModuleManager
 * @see MessageBus
 */
class Module {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~Module() = default;
    
    /**
     * @name Module Lifecycle Methods
     * @{
     */
    
    /**
     * @brief Initialize the module
     * 
     * This method is called during module initialization. It should perform any
     * necessary setup, resource allocation, and validation.
     * 
     * @return true if initialization was successful, false otherwise
     * @note This method is called before start() and should not start any threads or services
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Start the module
     * 
     * This method is called to start the module's main functionality. It should
     * start any threads, services, or other active components.
     * 
     * @note This method is called after initialize() and should be idempotent
     */
    virtual void start() = 0;
    
    /**
     * @brief Stop the module
     * 
     * This method is called to stop the module's main functionality. It should
     * gracefully stop any threads, services, or other active components.
     * 
     * @note This method should be idempotent and should not deallocate resources
     */
    virtual void stop() = 0;
    
    /**
     * @brief Shutdown the module
     * 
     * This method is called during module shutdown. It should perform cleanup,
     * deallocate resources, and prepare the module for destruction.
     * 
     * @note This method is called after stop() and before destruction
     */
    virtual void shutdown() = 0;
    
    /** @} */
    
    /**
     * @name Module Identification Methods
     * @{
     */
    
    /**
     * @brief Get the module name
     * 
     * @return The unique name of this module
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Get the module version
     * 
     * @return The version string of this module
     */
    virtual std::string getVersion() const = 0;
    
    /**
     * @brief Get module dependencies
     * 
     * @return Vector of module names that this module depends on
     */
    virtual std::vector<std::string> getDependencies() const = 0;
    
    /** @} */
    
    /**
     * @name Module Status Methods
     * @{
     */
    
    /**
     * @brief Check if the module is running
     * 
     * @return true if the module is currently running, false otherwise
     */
    virtual bool isRunning() const = 0;
    
    /**
     * @brief Get the current status of the module
     * 
     * @return A human-readable status string describing the module's current state
     */
    virtual std::string getStatus() const = 0;
    
    /** @} */
    
    /**
     * @name Configuration and Message Handling
     * @{
     */
    
    /**
     * @brief Configure the module
     * 
     * @param config A map of configuration key-value pairs
     * @return true if configuration was successful, false otherwise
     */
    virtual bool configure(const std::map<std::string, std::string>& config) = 0;
    
    /**
     * @brief Handle incoming messages
     * 
     * This method is called when a message is received on a topic that this
     * module has subscribed to.
     * 
     * @param topic The topic the message was published on
     * @param message The message payload
     */
    virtual void onMessage(const std::string& topic, const std::string& message) = 0;
    
    /** @} */
    
    /**
     * @name Accessor Methods
     * @{
     */
    
    /**
     * @brief Set the module manager reference
     * 
     * @param manager Pointer to the module manager
     */
    void setModuleManager(ModuleManager* manager) { moduleManager_ = manager; }
    
    /**
     * @brief Get the module manager reference
     * 
     * @return Pointer to the module manager, or nullptr if not set
     */
    ModuleManager* getModuleManager() const { return moduleManager_; }
    
    /**
     * @brief Set the message bus reference
     * 
     * @param bus Pointer to the message bus
     */
    void setMessageBus(MessageBus* bus) { messageBus_ = bus; }
    
    /**
     * @brief Get the message bus reference
     * 
     * @return Pointer to the message bus, or nullptr if not set
     */
    MessageBus* getMessageBus() const { return messageBus_; }
    
    /** @} */

protected:
    /** @brief Reference to the module manager */
    ModuleManager* moduleManager_ = nullptr;
    
    /** @brief Reference to the message bus */
    MessageBus* messageBus_ = nullptr;
    
    /** @brief Flag indicating if the module is currently running */
    bool running_ = false;
};

} // namespace swarm

#endif // MODULE_H
