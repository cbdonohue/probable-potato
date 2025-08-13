#include "../../../include/modules/health_monitor_module.h"
#include "../../../include/core/message_bus.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>

namespace swarm {

HealthMonitorModule::HealthMonitorModule() 
    : shouldStop_(false), totalChecks_(0), failedChecks_(0),
      defaultTimeoutMs_(5000), defaultIntervalMs_(30000), maxFailures_(3),
      enableNotifications_(true) {
}

HealthMonitorModule::~HealthMonitorModule() {
    shutdown();
}

bool HealthMonitorModule::initialize() {
    return true;
}

void HealthMonitorModule::start() {
    if (running_) return;
    
    running_ = true;
    shouldStop_ = false;
    monitoringThread_ = std::thread(&HealthMonitorModule::monitoringLoop, this);
    
    std::cout << "Health Monitor started" << std::endl;
}

void HealthMonitorModule::stop() {
    if (!running_) return;
    
    shouldStop_ = true;
    running_ = false;
    
    if (monitoringThread_.joinable()) {
        monitoringThread_.join();
    }
    
    std::cout << "Health Monitor stopped" << std::endl;
}

void HealthMonitorModule::shutdown() {
    stop();
}

std::string HealthMonitorModule::getStatus() const {
    std::ostringstream status;
    status << "Health Monitor (running: " << (running_ ? "yes" : "no")
           << ", checks: " << totalChecks_.load() << ", failed: " << failedChecks_.load()
           << ", success rate: " << getSuccessRate() * 100 << "%)";
    return status.str();
}

bool HealthMonitorModule::configure(const std::map<std::string, std::string>& config) {
    auto it = config.find("default_timeout_ms");
    if (it != config.end()) {
        defaultTimeoutMs_ = std::stoi(it->second);
    }
    
    it = config.find("default_interval_ms");
    if (it != config.end()) {
        defaultIntervalMs_ = std::stoi(it->second);
    }
    
    it = config.find("max_failures");
    if (it != config.end()) {
        maxFailures_ = std::stoi(it->second);
    }
    
    it = config.find("enable_notifications");
    if (it != config.end()) {
        enableNotifications_ = (it->second == "true" || it->second == "1");
    }
    
    return true;
}

void HealthMonitorModule::onMessage(const std::string& topic, const std::string& message) {
    if (topic == "health.check") {
        // Handle manual health check requests
        performHealthCheck(message);
    } else if (topic == "health.add") {
        // Handle dynamic health check additions
        // Parse message as JSON and add health check
    }
}

void HealthMonitorModule::addHealthCheck(const HealthCheckConfig& config) {
    std::lock_guard<std::mutex> lock(healthChecksMutex_);
    healthChecks_[config.moduleName] = config;
    
    // Initialize health status
    std::lock_guard<std::mutex> statusLock(healthStatusMutex_);
    healthStatus_[config.moduleName] = {
        config.moduleName, true, "Initialized", 
        std::chrono::system_clock::now(), 
        std::chrono::milliseconds(0), ""
    };
    failureCounts_[config.moduleName] = 0;
}

void HealthMonitorModule::removeHealthCheck(const std::string& moduleName) {
    std::lock_guard<std::mutex> lock(healthChecksMutex_);
    healthChecks_.erase(moduleName);
    
    std::lock_guard<std::mutex> statusLock(healthStatusMutex_);
    healthStatus_.erase(moduleName);
    failureCounts_.erase(moduleName);
}

void HealthMonitorModule::updateHealthCheck(const HealthCheckConfig& config) {
    std::lock_guard<std::mutex> lock(healthChecksMutex_);
    healthChecks_[config.moduleName] = config;
}

HealthCheckResult HealthMonitorModule::getModuleHealth(const std::string& moduleName) const {
    std::lock_guard<std::mutex> lock(healthStatusMutex_);
    auto it = healthStatus_.find(moduleName);
    return (it != healthStatus_.end()) ? it->second : HealthCheckResult{};
}

std::map<std::string, HealthCheckResult> HealthMonitorModule::getAllHealthStatus() const {
    std::lock_guard<std::mutex> lock(healthStatusMutex_);
    return healthStatus_;
}

bool HealthMonitorModule::isModuleHealthy(const std::string& moduleName) const {
    std::lock_guard<std::mutex> lock(healthStatusMutex_);
    auto it = healthStatus_.find(moduleName);
    return (it != healthStatus_.end() && it->second.healthy);
}

HealthCheckResult HealthMonitorModule::performHealthCheck(const std::string& moduleName) {
    std::lock_guard<std::mutex> lock(healthChecksMutex_);
    auto it = healthChecks_.find(moduleName);
    if (it == healthChecks_.end()) {
        return {moduleName, false, "No health check configured", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0), 
                "Module not found"};
    }
    
    return performHealthCheck(it->second);
}

void HealthMonitorModule::performAllHealthChecks() {
    std::lock_guard<std::mutex> lock(healthChecksMutex_);
    for (const auto& [name, config] : healthChecks_) {
        auto result = performHealthCheck(config);
        updateHealthStatus(name, result);
    }
}

size_t HealthMonitorModule::getTotalChecks() const {
    return totalChecks_.load();
}

size_t HealthMonitorModule::getFailedChecks() const {
    return failedChecks_.load();
}

double HealthMonitorModule::getSuccessRate() const {
    size_t total = totalChecks_.load();
    if (total == 0) return 1.0;
    return static_cast<double>(total - failedChecks_.load()) / total;
}

void HealthMonitorModule::monitoringLoop() {
    while (!shouldStop_) {
        performAllHealthChecks();
        
        // Sleep for the monitoring interval
        std::this_thread::sleep_for(std::chrono::milliseconds(defaultIntervalMs_));
    }
}

HealthCheckResult HealthMonitorModule::performHealthCheck(const HealthCheckConfig& config) {
    auto startTime = std::chrono::high_resolution_clock::now();
    totalChecks_++;
    
    HealthCheckResult result;
    result.moduleName = config.moduleName;
    result.lastCheck = std::chrono::system_clock::now();
    
    try {
        if (config.checkType == "http") {
            result = performHttpHealthCheck(config);
        } else if (config.checkType == "tcp") {
            result = performTcpHealthCheck(config);
        } else {
            result.healthy = false;
            result.status = "Unknown check type";
            result.errorMessage = "Unsupported check type: " + config.checkType;
        }
    } catch (const std::exception& e) {
        result.healthy = false;
        result.status = "Error";
        result.errorMessage = e.what();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.responseTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (!result.healthy) {
        failedChecks_++;
    }
    
    return result;
}

HealthCheckResult HealthMonitorModule::performHttpHealthCheck(const HealthCheckConfig& config) {
    // Simple HTTP health check implementation
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return {config.moduleName, false, "Socket creation failed", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0), 
                "Failed to create socket"};
    }
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); // Default HTTP port
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        close(sock);
        return {config.moduleName, false, "Invalid address", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0), 
                "Invalid address"};
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return {config.moduleName, false, "Connection failed", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0), 
                "Connection failed"};
    }
    
    close(sock);
    return {config.moduleName, true, "Healthy", 
            std::chrono::system_clock::now(), std::chrono::milliseconds(0), ""};
}

HealthCheckResult HealthMonitorModule::performTcpHealthCheck(const HealthCheckConfig& config) {
    // Simple TCP health check implementation
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return {config.moduleName, false, "Socket creation failed", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0), 
                "Failed to create socket"};
    }
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); // Default port
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        close(sock);
        return {config.moduleName, false, "Invalid address", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0), 
                "Invalid address"};
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return {config.moduleName, false, "Connection failed", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0), 
                "Connection failed"};
    }
    
    close(sock);
    return {config.moduleName, true, "Healthy", 
            std::chrono::system_clock::now(), std::chrono::milliseconds(0), ""};
}

void HealthMonitorModule::updateHealthStatus(const std::string& moduleName, const HealthCheckResult& result) {
    std::lock_guard<std::mutex> lock(healthStatusMutex_);
    
    bool wasHealthy = healthStatus_[moduleName].healthy;
    healthStatus_[moduleName] = result;
    
    // Update failure count
    if (!result.healthy) {
        failureCounts_[moduleName]++;
    } else {
        failureCounts_[moduleName] = 0;
    }
    
    // Notify if health status changed
    if (wasHealthy != result.healthy && enableNotifications_) {
        notifyHealthChange(moduleName, result.healthy);
    }
}

void HealthMonitorModule::notifyHealthChange(const std::string& moduleName, bool healthy) {
    if (messageBus_) {
        std::ostringstream message;
        message << "{\"module\": \"" << moduleName << "\", \"healthy\": " << (healthy ? "true" : "false") << "}";
        messageBus_->publish("health.status_change", message.str());
    }
}

} // namespace swarm
