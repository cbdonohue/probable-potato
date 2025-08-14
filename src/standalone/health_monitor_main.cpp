#include "../include/modules/health_monitor_module.h"
#include <iostream>
#include <signal.h>

using namespace swarm;

HealthMonitorModule* g_monitor = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down Health Monitor..." << std::endl;
    if (g_monitor) {
        g_monitor->stop();
    }
    exit(0);
}

int main() {
    std::cout << "🚀 Starting Health Monitor Module (Standalone)" << std::endl;

    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Create and configure health monitor module
        auto monitor = std::make_unique<HealthMonitorModule>();
        g_monitor = monitor.get();

        // Configure the monitor
        std::map<std::string, std::string> config = {
            {"default_timeout_ms", "5000"},
            {"default_interval_ms", "10000"},  // Check every 10 seconds
            {"max_failures", "3"},
            {"enable_notifications", "true"}
        };

        if (!monitor->configure(config)) {
            std::cerr << "❌ Failed to configure Health Monitor module" << std::endl;
            return 1;f
        }

        if (!monitor->initialize()) {
            std::cerr << "❌ Failed to initialize Health Monitor module" << std::endl;
            return 1;
        }

        std::cout << "✅ Health Monitor module initialized successfully" << std::endl;

        // Add health checks for the API service
        HealthCheckConfig apiCheck = {
            "api-service", "http", "http://swarm-app_api:8083/health", 5000, 10000, 3
        };
        monitor->addHealthCheck(apiCheck);

        HealthCheckConfig mainCheck = {
            "main-endpoint", "http", "http://swarm-app_api:8083/", 5000, 15000, 3
        };
        monitor->addHealthCheck(mainCheck);

        std::cout << "📋 Added health checks for:" << std::endl;
        std::cout << "   - Web service health (web:8080/health)" << std::endl;
        std::cout << "   - Main endpoint (web:8080/)" << std::endl;

        // Start the monitor
        monitor->start();

        std::cout << "🎯 Health Monitor is running..." << std::endl;
        std::cout << "🔧 Press Ctrl+C to stop" << std::endl;

        // Keep the monitor running
        while (monitor->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            // Print status every 5 seconds
            std::cout << "\n📈 Health Monitor Status: " << monitor->getStatus() << std::endl;
            
            // Print health status for monitored services
            auto healthStatus = monitor->getAllHealthStatus();
            for (const auto& [name, result] : healthStatus) {
                std::cout << "   " << name << ": " << (result.healthy ? "✅ Healthy" : "❌ Unhealthy") 
                         << " (" << result.status << ")" << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
