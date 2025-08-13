#include "../../include/core/module_manager.h"
#include <iostream>
#include <algorithm>

namespace swarm {

ModuleManager::ModuleManager() : initialized_(false) {
    messageBus_.start();
}

ModuleManager::~ModuleManager() {
    shutdownAllModules();
    messageBus_.stop();
}

void ModuleManager::registerModule(const std::string& name, ModuleFactory factory) {
    modules_[name] = {nullptr, factory, false, false, {}};
}

void ModuleManager::unregisterModule(const std::string& name) {
    auto it = modules_.find(name);
    if (it != modules_.end()) {
        if (it->second.running) {
            stopModule(name);
        }
        if (it->second.loaded) {
            unloadModule(name);
        }
        modules_.erase(it);
    }
}

bool ModuleManager::loadModule(const std::string& name, const std::map<std::string, std::string>& config) {
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        std::cerr << "Module '" << name << "' not registered" << std::endl;
        return false;
    }
    
    if (it->second.loaded) {
        std::cerr << "Module '" << name << "' already loaded" << std::endl;
        return false;
    }
    
    try {
        auto module = it->second.factory();
        if (!module) {
            std::cerr << "Failed to create module '" << name << "'" << std::endl;
            return false;
        }
        
        module->setModuleManager(this);
        module->setMessageBus(&messageBus_);
        
        if (!module->configure(config)) {
            std::cerr << "Failed to configure module '" << name << "'" << std::endl;
            return false;
        }
        
        if (!module->initialize()) {
            std::cerr << "Failed to initialize module '" << name << "'" << std::endl;
            return false;
        }
        
        it->second.module = std::move(module);
        it->second.loaded = true;
        it->second.config = config;
        
        std::cout << "Module '" << name << "' loaded successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading module '" << name << "': " << e.what() << std::endl;
        return false;
    }
}

bool ModuleManager::unloadModule(const std::string& name) {
    auto it = modules_.find(name);
    if (it == modules_.end() || !it->second.loaded) {
        return false;
    }
    
    if (it->second.running) {
        stopModule(name);
    }
    
    it->second.module->shutdown();
    it->second.module.reset();
    it->second.loaded = false;
    
    std::cout << "Module '" << name << "' unloaded" << std::endl;
    return true;
}

bool ModuleManager::startModule(const std::string& name) {
    auto it = modules_.find(name);
    if (it == modules_.end() || !it->second.loaded) {
        return false;
    }
    
    if (it->second.running) {
        return true;
    }
    
    try {
        it->second.module->start();
        it->second.running = true;
        std::cout << "Module '" << name << "' started" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error starting module '" << name << "': " << e.what() << std::endl;
        return false;
    }
}

bool ModuleManager::stopModule(const std::string& name) {
    auto it = modules_.find(name);
    if (it == modules_.end() || !it->second.loaded || !it->second.running) {
        return false;
    }
    
    try {
        it->second.module->stop();
        it->second.running = false;
        std::cout << "Module '" << name << "' stopped" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error stopping module '" << name << "': " << e.what() << std::endl;
        return false;
    }
}

void ModuleManager::startAllModules() {
    for (auto& [name, info] : modules_) {
        if (info.loaded && !info.running) {
            startModule(name);
        }
    }
}

void ModuleManager::stopAllModules() {
    for (auto& [name, info] : modules_) {
        if (info.loaded && info.running) {
            stopModule(name);
        }
    }
}

void ModuleManager::shutdownAllModules() {
    stopAllModules();
    for (auto& [name, info] : modules_) {
        if (info.loaded) {
            unloadModule(name);
        }
    }
}

Module* ModuleManager::getModule(const std::string& name) const {
    auto it = modules_.find(name);
    return (it != modules_.end() && it->second.loaded) ? it->second.module.get() : nullptr;
}

std::vector<std::string> ModuleManager::getLoadedModules() const {
    std::vector<std::string> loaded;
    for (const auto& [name, info] : modules_) {
        if (info.loaded) {
            loaded.push_back(name);
        }
    }
    return loaded;
}

std::vector<std::string> ModuleManager::getRunningModules() const {
    std::vector<std::string> running;
    for (const auto& [name, info] : modules_) {
        if (info.loaded && info.running) {
            running.push_back(name);
        }
    }
    return running;
}

bool ModuleManager::resolveDependencies(const std::string& moduleName) {
    auto module = getModule(moduleName);
    if (!module) {
        return false;
    }
    
    auto dependencies = module->getDependencies();
    return checkDependencies(dependencies);
}

std::vector<std::string> ModuleManager::getModuleDependencies(const std::string& moduleName) const {
    auto module = getModule(moduleName);
    return module ? module->getDependencies() : std::vector<std::string>{};
}

std::map<std::string, std::string> ModuleManager::getModuleStatuses() const {
    std::map<std::string, std::string> statuses;
    for (const auto& [name, info] : modules_) {
        if (info.loaded) {
            statuses[name] = info.module->getStatus();
        }
    }
    return statuses;
}

bool ModuleManager::isModuleRunning(const std::string& name) const {
    auto it = modules_.find(name);
    return (it != modules_.end() && it->second.loaded && it->second.running);
}

bool ModuleManager::checkDependencies(const std::vector<std::string>& dependencies) const {
    for (const auto& dep : dependencies) {
        if (!isModuleRunning(dep)) {
            std::cerr << "Dependency '" << dep << "' is not running" << std::endl;
            return false;
        }
    }
    return true;
}

void ModuleManager::loadModuleDependencies(const std::string& moduleName) {
    auto module = getModule(moduleName);
    if (!module) {
        return;
    }
    
    auto dependencies = module->getDependencies();
    for (const auto& dep : dependencies) {
        if (!getModule(dep)) {
            loadModule(dep);
        }
    }
}

} // namespace swarm
