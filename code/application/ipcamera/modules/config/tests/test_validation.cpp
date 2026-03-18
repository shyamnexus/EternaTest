#include "ipcam/config_loader.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

bool test_valid_config() {
    std::cout << "\n=== Testing Valid Configuration ===" << std::endl;
    
    auto& cfg = ipcam::config::ConfigLoader::Instance();
    
    // Find configs directory
    const char* env = std::getenv("IPCAM_CONFIG_DIR");
    std::vector<std::string> candidates;
    if(env) candidates.push_back(std::string(env));
    candidates.push_back("./configs");
    candidates.push_back("../configs");
    candidates.push_back("../../configs");
    candidates.push_back("../../../configs");
    candidates.push_back("../../../../configs");

    std::string chosen;
    for(auto &c: candidates){
        try{
            if(fs::is_directory(c)){
                chosen = c;
                break;
            }
        }catch(...){}
    }

    if(chosen.empty()){
        std::cout << "No configs directory found" << std::endl;
        return false;
    }

    std::cout << "Using configs directory: " << chosen << std::endl;

    if(!cfg.LoadDirectory(chosen)){
        std::cout << "Failed to load configuration directory" << std::endl;
        return false;
    }

    std::string error;
    bool isValid = cfg.Validate(error);
    
    if(isValid) {
        std::cout << "✓ Configuration validation passed!" << std::endl;
        return true;
    } else {
        std::cout << "✗ Configuration validation failed: " << error << std::endl;
        return false;
    }
}

bool test_invalid_network_config() {
    std::cout << "\n=== Testing Invalid Network Configuration ===" << std::endl;
    
    auto& cfg = ipcam::config::ConfigLoader::Instance();
    
    // Create a temporary config with invalid port
    YAML::Node invalidConfig;
    invalidConfig["network"]["http"]["port"] = 99999; // Invalid port
    invalidConfig["storage"]["database"]["user_management"]["path"] = "/tmp/test.db";
    invalidConfig["storage"]["database"]["recordings"]["path"] = "/tmp/recordings.db";
    
    std::vector<std::string> tempFiles;
    std::string tempFile = "/tmp/test_invalid_config.yaml";
    
    std::ofstream file(tempFile);
    if(!file.is_open()) {
        std::cout << "Failed to create temp file" << std::endl;
        return false;
    }
    file << invalidConfig;
    file.close();
    
    tempFiles.push_back(tempFile);
    
    if(!cfg.LoadFiles(tempFiles)){
        std::cout << "Failed to load test configuration" << std::endl;
        fs::remove(tempFile);
        return false;
    }

    std::string error;
    bool isValid = cfg.Validate(error);
    
    // Clean up
    fs::remove(tempFile);
    
    if(!isValid && error.find("network.http.port must be between 1 and 65535") != std::string::npos) {
        std::cout << "✓ Correctly detected invalid port: " << error << std::endl;
        return true;
    } else {
        std::cout << "✗ Failed to detect invalid port configuration" << std::endl;
        return false;
    }
}

bool test_missing_required_config() {
    std::cout << "\n=== Testing Missing Required Configuration ===" << std::endl;
    
    auto& cfg = ipcam::config::ConfigLoader::Instance();
    
    // Create a config missing required fields
    YAML::Node incompleteConfig;
    incompleteConfig["network"]["http"]["port"] = 8080;
    // Missing storage.database paths
    
    std::vector<std::string> tempFiles;
    std::string tempFile = "/tmp/test_incomplete_config.yaml";
    
    std::ofstream file(tempFile);
    if(!file.is_open()) {
        std::cout << "Failed to create temp file" << std::endl;
        return false;
    }
    file << incompleteConfig;
    file.close();
    
    tempFiles.push_back(tempFile);
    
    if(!cfg.LoadFiles(tempFiles)){
        std::cout << "Failed to load test configuration" << std::endl;
        fs::remove(tempFile);
        return false;
    }

    std::string error;
    bool isValid = cfg.Validate(error);
    
    // Clean up
    fs::remove(tempFile);
    
    if(!isValid && error.find("missing storage.database.user_management.path") != std::string::npos) {
        std::cout << "✓ Correctly detected missing storage config: " << error << std::endl;
        return true;
    } else {
        std::cout << "✗ Failed to detect missing storage configuration. Error: " << error << std::endl;
        return false;
    }
}

bool test_invalid_media_config() {
    std::cout << "\n=== Testing Invalid Media Configuration ===" << std::endl;
    
    auto& cfg = ipcam::config::ConfigLoader::Instance();
    
    // Create a config with invalid media settings
    YAML::Node invalidMediaConfig;
    invalidMediaConfig["network"]["http"]["port"] = 8080;
    invalidMediaConfig["storage"]["database"]["user_management"]["path"] = "/tmp/test.db";
    invalidMediaConfig["storage"]["database"]["recordings"]["path"] = "/tmp/recordings.db";
    invalidMediaConfig["media"]["video1"]["width"] = 1920;
    invalidMediaConfig["media"]["video1"]["height"] = 1080;
    invalidMediaConfig["media"]["video1"]["fps"] = 150; // Invalid FPS > 120
    invalidMediaConfig["media"]["video1"]["bitrate"] = 1000000;
    invalidMediaConfig["media"]["video1"]["codec"] = "h264";
    
    std::vector<std::string> tempFiles;
    std::string tempFile = "/tmp/test_invalid_media_config.yaml";
    
    std::ofstream file(tempFile);
    if(!file.is_open()) {
        std::cout << "Failed to create temp file" << std::endl;
        return false;
    }
    file << invalidMediaConfig;
    file.close();
    
    tempFiles.push_back(tempFile);
    
    if(!cfg.LoadFiles(tempFiles)){
        std::cout << "Failed to load test configuration" << std::endl;
        fs::remove(tempFile);
        return false;
    }

    std::string error;
    bool isValid = cfg.Validate(error);
    
    // Clean up
    fs::remove(tempFile);
    
    if(!isValid && error.find("fps must be between 1 and 120") != std::string::npos) {
        std::cout << "✓ Correctly detected invalid media config: " << error << std::endl;
        return true;
    } else {
        std::cout << "✗ Failed to detect invalid media configuration. Error: " << error << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Running ConfigLoader Validation Tests" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Valid configuration
    total++;
    if(test_valid_config()) passed++;
    
    // Test 2: Invalid network configuration
    total++;
    if(test_invalid_network_config()) passed++;
    
    // Test 3: Missing required configuration
    total++;
    if(test_missing_required_config()) passed++;
    
    // Test 4: Invalid media configuration
    total++;
    if(test_invalid_media_config()) passed++;
    
    std::cout << "\n=====================================" << std::endl;
    std::cout << "Test Results: " << passed << "/" << total << " passed" << std::endl;
    
    if(passed == total) {
        std::cout << "🎉 All validation tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some validation tests failed!" << std::endl;
        return 1;
    }
}
