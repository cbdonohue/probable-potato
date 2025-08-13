/**
 * @file health_monitor_module.h
 * @brief Health monitoring module for system health tracking
 * @author SwarmApp Development Team
 * @version 1.0.0
 */

#ifndef HEALTH_MONITOR_MODULE_H
#define HEALTH_MONITOR_MODULE_H

#include "../core/module.h"
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

namespace swarm {

/**
 * @brief Health check result structure
 * 
 * Contains the result of a health check operation
 */
struct HealthCheckResult {
    std::string moduleName;                               ///< Name of the module being checked
    bool healthy;                                         ///< Whether the module is healthy
    std::string status;                                   ///< Human-readable status message
    std::chrono::system_clock::time_point lastCheck;     ///< Timestamp of the last check
    std::chrono::milliseconds responseTime;              ///< Response time of the health check
    std::string errorMessage;                            ///< Error message if the check failed
};

/**
 * @brief Health check configuration structure
 * 
 * Defines how to perform health checks for a module
 */
struct HealthCheckConfig {
    std::string moduleName;                              ///< Name of the module to monitor
    std::string checkType;                               ///< Type of check: "http", "tcp", "custom"
    std::string endpoint;                                ///< Endpoint to check (URL, host:port, etc.)
    int timeoutMs;                                       ///< Timeout in milliseconds
    int intervalMs;                                      ///< Check interval in milliseconds
    int maxFailures;                                     ///< Maximum consecutive failures before marking unhealthy
};

/**
 * @brief Health Monitor Module
 * 
 * The HealthMonitorModule provides comprehensive health monitoring capabilities
 * for the SwarmApp system. It continuously monitors the health of all modules
 * and provides real-time status information.
 * 
 * Features:
 * - Multiple health check types (HTTP, TCP, custom)
 * - Configurable check intervals and timeouts
 * - Failure tracking and threshold management
 * - Real-time health status reporting
 * - Health change notifications
 * - Performance metrics and statistics
 * 
 * @note This module runs in its own thread and is thread-safe
 * @see Module
 * @see HealthCheckResult
 * @see HealthCheckConfig
 */
class HealthMonitorModule : public Module {
public:
    /**
     * @brief Constructor
     * 
     * Initializes the health monitor module with default configuration
     */
    HealthMonitorModule();
    
    /**
     * @brief Destructor
     * 
     * Stops monitoring and cleans up resources
     */
    ~HealthMonitorModule() override;
    
    /**
     * @name Module Interface Implementation
     * @{
     */
    
    /**
     * @brief Initialize the health monitor
     * 
     * Sets up the monitoring system and initializes configuration.
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool initialize() override;
    
    /**
     * @brief Start the health monitor
     * 
     * Starts the monitoring thread and begins health checks.
     */
    void start() override;
    
    /**
     * @brief Stop the health monitor
     * 
     * Stops the monitoring thread and halts health checks.
     */
    void stop() override;
    
    /**
     * @brief Shutdown the health monitor
     * 
     * Performs complete cleanup of monitoring resources.
     */
    void shutdown() override;
    
    /**
     * @brief Get the module name
     * 
     * @return The module name: "health-monitor"
     */
    std::string getName() const override { return "health-monitor"; }
    
    /**
     * @brief Get the module version
     * 
     * @return The module version: "1.0.0"
     */
    std::string getVersion() const override { return "1.0.0"; }
    
    /**
     * @brief Get module dependencies
     * 
     * @return Empty vector (no dependencies)
     */
    std::vector<std::string> getDependencies() const override { return {}; }
    
    /**
     * @brief Check if the monitor is running
     * 
     * @return true if the monitor is running, false otherwise
     */
    bool isRunning() const override { return running_; }
    
    /**
     * @brief Get the monitor status
     * 
     * @return A human-readable status string
     */
    std::string getStatus() const override;
    
    /**
     * @brief Configure the health monitor
     * 
     * @param config Configuration parameters
     * @return true if configuration was successful, false otherwise
     */
    bool configure(const std::map<std::string, std::string>& config) override;
    
    /**
     * @brief Handle incoming messages
     * 
     * Processes messages from the message bus.
     * 
     * @param topic The message topic
     * @param message The message payload
     */
    void onMessage(const std::string& topic, const std::string& message) override;
    
    /** @} */
    
    /**
     * @name Health Monitoring Methods
     * @{
     */
    
    /**
     * @brief Add a health check configuration
     * 
     * @param config The health check configuration to add
     */
    void addHealthCheck(const HealthCheckConfig& config);
    
    /**
     * @brief Remove a health check configuration
     * 
     * @param moduleName The name of the module to stop monitoring
     */
    void removeHealthCheck(const std::string& moduleName);
    
    /**
     * @brief Update a health check configuration
     * 
     * @param config The updated health check configuration
     */
    void updateHealthCheck(const HealthCheckConfig& config);
    
    /** @} */
    
    /**
     * @name Health Status Queries
     * @{
     */
    
    /**
     * @brief Get health status for a specific module
     * 
     * @param moduleName The name of the module
     * @return The health check result for the module
     */
    HealthCheckResult getModuleHealth(const std::string& moduleName) const;
    
    /**
     * @brief Get health status for all monitored modules
     * 
     * @return Map of module names to their health check results
     */
    std::map<std::string, HealthCheckResult> getAllHealthStatus() const;
    
    /**
     * @brief Check if a module is healthy
     * 
     * @param moduleName The name of the module
     * @return true if the module is healthy, false otherwise
     */
    bool isModuleHealthy(const std::string& moduleName) const;
    
    /** @} */
    
    /**
     * @name Manual Health Checks
     * @{
     */
    
    /**
     * @brief Perform a health check for a module
     * 
     * @param moduleName The name of the module to check
     * @return The health check result
     */
    HealthCheckResult performHealthCheck(const std::string& moduleName);
    
    /**
     * @brief Perform a health check with custom configuration
     * 
     * @param config The health check configuration to use
     * @return The health check result
     */
    HealthCheckResult performHealthCheck(const HealthCheckConfig& config);
    
    /**
     * @brief Perform health checks for all monitored modules
     */
    void performAllHealthChecks();
    
    /** @} */
    
    /**
     * @name Statistics Methods
     * @{
     */
    
    /**
     * @brief Get the total number of health checks performed
     * 
     * @return The total check count
     */
    size_t getTotalChecks() const;
    
    /**
     * @brief Get the number of failed health checks
     * 
     * @return The failed check count
     */
    size_t getFailedChecks() const;
    
    /**
     * @brief Get the health check success rate
     * 
     * @return Success rate as a percentage (0.0 to 100.0)
     */
    double getSuccessRate() const;
    
    /** @} */

private:
    /**
     * @brief Main monitoring loop
     * 
     * Runs in a separate thread and performs periodic health checks
     */
    void monitoringLoop();
    
    /**
     * @brief Perform an HTTP health check
     * 
     * @param config The health check configuration
     * @return The health check result
     */
    HealthCheckResult performHttpHealthCheck(const HealthCheckConfig& config);
    
    /**
     * @brief Perform a TCP health check
     * 
     * @param config The health check configuration
     * @return The health check result
     */
    HealthCheckResult performTcpHealthCheck(const HealthCheckConfig& config);
    
    /**
     * @brief Update health status for a module
     * 
     * @param moduleName The name of the module
     * @param result The health check result
     */
    void updateHealthStatus(const std::string& moduleName, const HealthCheckResult& result);
    
    /**
     * @brief Notify about health status changes
     * 
     * @param moduleName The name of the module
     * @param healthy Whether the module is now healthy
     */
    void notifyHealthChange(const std::string& moduleName, bool healthy);
    
    std::thread monitoringThread_;                         ///< Monitoring thread
    std::atomic<bool> shouldStop_;                         ///< Flag to stop monitoring
    std::atomic<size_t> totalChecks_;                      ///< Total health checks performed
    std::atomic<size_t> failedChecks_;                     ///< Failed health checks count
    
    std::map<std::string, HealthCheckConfig> healthChecks_; ///< Health check configurations
    std::map<std::string, HealthCheckResult> healthStatus_; ///< Current health status
    std::map<std::string, int> failureCounts_;             ///< Consecutive failure counts
    
    mutable std::mutex healthChecksMutex_;                 ///< Mutex for health checks
    mutable std::mutex healthStatusMutex_;                 ///< Mutex for health status
    
    // Configuration
    int defaultTimeoutMs_;                                 ///< Default timeout in milliseconds
    int defaultIntervalMs_;                                ///< Default check interval in milliseconds
    int maxFailures_;                                      ///< Maximum consecutive failures
    bool enableNotifications_;                             ///< Enable health change notifications
};

} // namespace swarm

#endif // HEALTH_MONITOR_MODULE_H
