#include "../../../include/modules/health_monitor_module.h"
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
            return 1;
        }

        if (!monitor->initialize()) {
            std::cerr << "❌ Failed to initialize Health Monitor module" << std::endl;
            return 1;
        }

        std::cout << "✅ Health Monitor module initialized successfully" << std::endl;

        // Add some example health checks
        HealthCheckConfig httpCheck = {
            "http-server", "http", "http://localhost:8080/health", 5000, 10000, 3
        };
        monitor->addHealthCheck(httpCheck);

        HealthCheckConfig tcpCheck = {
            "tcp-server", "tcp", "localhost:8080", 5000, 15000, 3
        };
        monitor->addHealthCheck(tcpCheck);

        std::cout << "📋 Added health checks for:" << std::endl;
        std::cout << "   - HTTP server (localhost:8080/health)" << std::endl;
        std::cout << "   - TCP server (localhost:8080)" << std::endl;

        // Start the monitor
        monitor->start();

        std::cout << "🎯 Health Monitor is running" << std::endl;
        std::cout << "📊 Monitoring interval: 10 seconds" << std::endl;
        std::cout << "🔧 Press Ctrl+C to stop" << std::endl;

        // Keep the monitor running and show status
        while (monitor->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            // Print status every 5 seconds
            std::cout << "\n📈 Health Status:" << std::endl;
            auto allHealth = monitor->getAllHealthStatus();
            for (const auto& [name, result] : allHealth) {
                std::cout << "   " << name << ": " 
                          << (result.healthy ? "✅ Healthy" : "❌ Unhealthy")
                          << " (" << result.status << ")" << std::endl;
            }
            
            std::cout << "📊 Statistics: " 
                      << monitor->getTotalChecks() << " total checks, "
                      << monitor->getFailedChecks() << " failed, "
                      << "Success rate: " << (monitor->getSuccessRate() * 100) << "%" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
