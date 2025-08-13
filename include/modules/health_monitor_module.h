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

// Health check result
struct HealthCheckResult {
    std::string moduleName;
    bool healthy;
    std::string status;
    std::chrono::system_clock::time_point lastCheck;
    std::chrono::milliseconds responseTime;
    std::string errorMessage;
};

// Health check configuration
struct HealthCheckConfig {
    std::string moduleName;
    std::string checkType;  // "http", "tcp", "custom"
    std::string endpoint;
    int timeoutMs;
    int intervalMs;
    int maxFailures;
};

// Health Monitor Module
class HealthMonitorModule : public Module {
public:
    HealthMonitorModule();
    ~HealthMonitorModule() override;
    
    // Module interface implementation
    bool initialize() override;
    void start() override;
    void stop() override;
    void shutdown() override;
    
    std::string getName() const override { return "health-monitor"; }
    std::string getVersion() const override { return "1.0.0"; }
    std::vector<std::string> getDependencies() const override { return {}; }
    
    bool isRunning() const override { return running_; }
    std::string getStatus() const override;
    
    bool configure(const std::map<std::string, std::string>& config) override;
    void onMessage(const std::string& topic, const std::string& message) override;
    
    // Health monitoring specific methods
    void addHealthCheck(const HealthCheckConfig& config);
    void removeHealthCheck(const std::string& moduleName);
    void updateHealthCheck(const HealthCheckConfig& config);
    
    // Health status queries
    HealthCheckResult getModuleHealth(const std::string& moduleName) const;
    std::map<std::string, HealthCheckResult> getAllHealthStatus() const;
    bool isModuleHealthy(const std::string& moduleName) const;
    
    // Manual health checks
    HealthCheckResult performHealthCheck(const std::string& moduleName);
    HealthCheckResult performHealthCheck(const HealthCheckConfig& config);
    void performAllHealthChecks();
    
    // Statistics
    size_t getTotalChecks() const;
    size_t getFailedChecks() const;
    double getSuccessRate() const;

private:
    void monitoringLoop();
    HealthCheckResult performHttpHealthCheck(const HealthCheckConfig& config);
    HealthCheckResult performTcpHealthCheck(const HealthCheckConfig& config);
    void updateHealthStatus(const std::string& moduleName, const HealthCheckResult& result);
    void notifyHealthChange(const std::string& moduleName, bool healthy);
    
    std::thread monitoringThread_;
    std::atomic<bool> shouldStop_;
    std::atomic<size_t> totalChecks_;
    std::atomic<size_t> failedChecks_;
    
    std::map<std::string, HealthCheckConfig> healthChecks_;
    std::map<std::string, HealthCheckResult> healthStatus_;
    std::map<std::string, int> failureCounts_;
    
    mutable std::mutex healthChecksMutex_;
    mutable std::mutex healthStatusMutex_;
    
    // Configuration
    int defaultTimeoutMs_;
    int defaultIntervalMs_;
    int maxFailures_;
    bool enableNotifications_;
};

} // namespace swarm

#endif // HEALTH_MONITOR_MODULE_H
