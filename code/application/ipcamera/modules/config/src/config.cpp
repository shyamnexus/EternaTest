#include "ipcam/config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstdint>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace ipcam {
namespace config {

namespace {
    std::map<std::string, json> g_cache;
    std::mutex g_mutex;
    std::string g_config_dir;
    json g_config_tree;      // Merged config: factory + user overrides (runtime state)
    json g_factory_tree;     // Factory defaults snapshot (immutable after boot)
    bool g_initialized = false;
}

// Compute user delta: returns a JSON object containing only the values in
// 'current' that differ from 'factory'. Recurses into objects so that
// unchanged sub-keys are not included in the output.
static json ComputeUserDelta(const json& factory, const json& current) {
    if (!factory.is_object() || !current.is_object()) {
        // Leaf or type mismatch — if different, the entire value is user-modified
        return (factory != current) ? current : json();
    }
    
    json delta = json::object();
    
    // Keys in current that are new or changed vs factory
    for (auto& [key, val] : current.items()) {
        if (!factory.contains(key)) {
            // Entirely new key added by user
            delta[key] = val;
        } else {
            json sub = ComputeUserDelta(factory[key], val);
            if (!sub.is_null() && !(sub.is_object() && sub.empty())) {
                delta[key] = sub;
            }
        }
    }
    
    return delta;
}

// Helper to parse array index from key like "channels[0]" -> ("channels", 0)
// Returns true if key has array index, false otherwise
static bool ParseArrayIndex(const std::string& key, std::string& baseName, size_t& index) {
    size_t bracketStart = key.find('[');
    size_t bracketEnd = key.find(']');
    
    if (bracketStart != std::string::npos && bracketEnd != std::string::npos && bracketEnd > bracketStart + 1) {
        baseName = key.substr(0, bracketStart);
        try {
            index = std::stoul(key.substr(bracketStart + 1, bracketEnd - bracketStart - 1));
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

// Helper to get nested value from JSON using dot notation with array support
// Supports paths like: "storage.channels[0].id", "network.dns[1]"
static json GetNestedValue(const json& j, const std::string& path) {
    json current = j;
    std::string key;
    size_t pos = 0;
    size_t lastPos = 0;
    
    while (pos != std::string::npos) {
        pos = path.find('.', lastPos);
        key = (pos == std::string::npos) ? path.substr(lastPos) : path.substr(lastPos, pos - lastPos);
        
        // Check if key has array index (e.g., "channels[0]")
        std::string baseName;
        size_t arrayIndex;
        if (ParseArrayIndex(key, baseName, arrayIndex)) {
            // First navigate to the array
            if (!current.contains(baseName)) {
                return json(); // Return null JSON
            }
            current = current[baseName];
            
            // Then access the array element
            if (!current.is_array() || arrayIndex >= current.size()) {
                return json(); // Return null JSON if not array or index out of bounds
            }
            current = current[arrayIndex];
        } else {
            // Regular object key
            if (!current.contains(key)) {
                return json(); // Return null JSON
            }
            current = current[key];
        }
        lastPos = pos + 1;
    }
    
    return current;
}

// Helper to set nested value using dot notation with array support
// Supports paths like: "storage.channels[0].id", "network.dns[1]"
static void SetNestedValue(json& j, const std::string& path, const json& value) {
    json* current = &j;
    std::string key;
    size_t pos = 0;
    size_t lastPos = 0;
    std::vector<std::string> keys;
    
    // Split path into keys
    while (pos != std::string::npos) {
        pos = path.find('.', lastPos);
        key = (pos == std::string::npos) ? path.substr(lastPos) : path.substr(lastPos, pos - lastPos);
        keys.push_back(key);
        lastPos = pos + 1;
    }
    
    // Navigate/create path (all but last key)
    for (size_t i = 0; i < keys.size() - 1; i++) {
        std::string baseName;
        size_t arrayIndex;
        if (ParseArrayIndex(keys[i], baseName, arrayIndex)) {
            // Key has array index - ensure array exists and has enough elements
            if (!current->contains(baseName) || !(*current)[baseName].is_array()) {
                (*current)[baseName] = json::array();
            }
            json& arr = (*current)[baseName];
            while (arr.size() <= arrayIndex) {
                arr.push_back(json::object());
            }
            current = &arr[arrayIndex];
        } else {
            // Regular object key
            if (!current->contains(keys[i]) || !(*current)[keys[i]].is_object()) {
                (*current)[keys[i]] = json::object();
            }
            current = &(*current)[keys[i]];
        }
    }
    
    // Set final value (handle array index in final key too)
    const std::string& finalKey = keys.back();
    std::string baseName;
    size_t arrayIndex;
    if (ParseArrayIndex(finalKey, baseName, arrayIndex)) {
        // Final key has array index
        if (!current->contains(baseName) || !(*current)[baseName].is_array()) {
            (*current)[baseName] = json::array();
        }
        json& arr = (*current)[baseName];
        while (arr.size() <= arrayIndex) {
            arr.push_back(json());
        }
        arr[arrayIndex] = value;
    } else {
        // Regular object key
        (*current)[finalKey] = value;
    }
}

// Helper function to load all JSON files from a directory
static void LoadConfigDirectory(const std::string& dirPath, json& target) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(dirPath)) {
        spdlog::warn("Config directory does not exist: {}", dirPath);
        return;
    }
    
    if (!fs::is_directory(dirPath)) {
        spdlog::warn("Config path is not a directory: {}", dirPath);
        return;
    }
    
    std::vector<fs::path> jsonFiles;
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            jsonFiles.push_back(entry.path());
        }
    }
    
    if (jsonFiles.empty()) {
        spdlog::warn("No JSON config files found in: {}", dirPath);
        return;
    }
    
    // Sort for deterministic loading order
    std::sort(jsonFiles.begin(), jsonFiles.end());
    
    spdlog::info("Loading {} config files from: {}", jsonFiles.size(), dirPath);
    
    for (const auto& filePath : jsonFiles) {
        std::ifstream file(filePath);
        if (file.is_open()) {
            try {
                json moduleConfig = json::parse(file);
                target.merge_patch(moduleConfig);
                spdlog::debug("Loaded config module: {}", filePath.filename().string());
            } catch (const json::exception& e) {
                spdlog::warn("Failed to parse {}: {}", filePath.string(), e.what());
            }
            file.close();
        } else {
            spdlog::warn("Failed to open config file: {}", filePath.string());
        }
    }
}

bool Init(const std::string& configDir, std::string& outError) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (g_initialized) {
        outError = "Config already initialized";
        return false;
    }
    
    g_config_dir = configDir;
    g_config_tree = json::object();
    
    spdlog::info("Initializing modular JSON config from: {}", configDir);
    
    // Load factory defaults from modular directory
    std::string factoryDir = configDir + "/config.factory.d";
    LoadConfigDirectory(factoryDir, g_config_tree);
    spdlog::info("Loaded factory configs from: {}", factoryDir);
    
    // Fallback: Load monolithic factory config if modular doesn't exist
    std::string factoryPath = configDir + "/config.factory.json";
    std::ifstream factoryFile(factoryPath);
    if (factoryFile.is_open()) {
        try {
            json factory = json::parse(factoryFile);
            g_config_tree.merge_patch(factory);
            spdlog::info("Merged monolithic factory config from: {}", factoryPath);
        } catch (const json::exception& e) {
            spdlog::warn("Failed to parse factory config: {}", e.what());
        }
        factoryFile.close();
    }
    
    // Load deployment-specific config overrides (config.d/).
    // These sit between factory defaults and per-user runtime changes.
    std::string userDir = configDir + "/config.d";
    LoadConfigDirectory(userDir, g_config_tree);
    if (std::filesystem::exists(userDir)) {
        spdlog::info("Loaded deployment configs from: {}", userDir);
    }
    
    // Snapshot the baseline (factory + deployment overrides) BEFORE merging
    // user runtime changes from config.json.  Save() computes a delta against
    // this baseline so that every value the user changed at runtime — even if
    // it happens to match a raw factory default — is persisted correctly.
    g_factory_tree = g_config_tree;
    spdlog::info("Config baseline snapshot captured ({} top-level keys)", g_factory_tree.size());
    
    // Load monolithic user config and merge (highest priority)
    // This file contains ONLY user-modified values (delta from factory).
    std::string userPath = configDir + "/config.json";
    std::ifstream userFile(userPath);
    if (userFile.is_open()) {
        try {
            json user = json::parse(userFile);
            g_config_tree.merge_patch(user);
            spdlog::info("Merged user config from: {} ({} top-level keys)", userPath, user.size());
        } catch (const json::exception& e) {
            spdlog::warn("Failed to parse user config: {}", e.what());
        }
        userFile.close();
    }
    
    g_initialized = true;
    spdlog::info("Config initialization complete with {} top-level keys", g_config_tree.size());
    return true;
}

void Shutdown() {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_cache.clear();
    g_config_tree = json::object();
    g_factory_tree = json::object();
    g_initialized = false;
    spdlog::info("Config module shutdown");
}

template<typename T>
T Get(const std::string& path, const T& defaultValue) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        spdlog::warn("Config not initialized, returning default for: {}", path);
        return defaultValue;
    }
    
    try {
        json value = GetNestedValue(g_config_tree, path);
        if (value.is_null()) {
            spdlog::debug("Config key not found, using default: {}", path);
            return defaultValue;
        }
        return value.get<T>();
    } catch (const json::exception& e) {
        spdlog::warn("Error getting config {}: {}, using default", path, e.what());
        return defaultValue;
    }
}

template<typename T>
bool Set(const std::string& path, const T& value) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        spdlog::error("Config not initialized, cannot set: {}", path);
        return false;
    }
    
    try {
        SetNestedValue(g_config_tree, path, value);
        spdlog::debug("Config set: {} = {}", path, json(value).dump());
        return true;
    } catch (const json::exception& e) {
        spdlog::error("Error setting config {}: {}", path, e.what());
        return false;
    }
}

template<typename T>
std::optional<T> GetOptional(const std::string& path) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        return std::nullopt;
    }
    
    try {
        json value = GetNestedValue(g_config_tree, path);
        if (value.is_null()) {
            return std::nullopt;
        }
        return value.get<T>();
    } catch (const json::exception&) {
        return std::nullopt;
    }
}

bool Has(const std::string& path) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        return false;
    }
    
    json value = GetNestedValue(g_config_tree, path);
    return !value.is_null();
}

bool Remove(const std::string& path) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        return false;
    }
    
    try {
        // Navigate to parent and remove key
        size_t lastDot = path.rfind('.');
        if (lastDot == std::string::npos) {
            g_config_tree.erase(path);
        } else {
            std::string parentPath = path.substr(0, lastDot);
            std::string key = path.substr(lastDot + 1);
            json parent = GetNestedValue(g_config_tree, parentPath);
            if (parent.is_object()) {
                parent.erase(key);
            }
        }
        return true;
    } catch (const json::exception& e) {
        spdlog::error("Error removing config {}: {}", path, e.what());
        return false;
    }
}

bool Save(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (!g_initialized) {
        spdlog::error("Config not initialized, cannot save");
        return false;
    }
    
    std::string savePath = filePath.empty() ? (g_config_dir + "/config.json") : filePath;
    std::string tempPath = savePath + ".tmp";
    
    try {
        // Compute user delta: only save values that differ from factory defaults.
        // This ensures factory config updates take effect on next boot for any
        // settings the user hasn't explicitly changed.
        json userDelta = ComputeUserDelta(g_factory_tree, g_config_tree);
        
        if (userDelta.empty() || (userDelta.is_object() && userDelta.size() == 0)) {
            // No user overrides — remove config.json if it exists so factory
            // defaults apply cleanly on next boot
            if (std::filesystem::exists(savePath)) {
                std::filesystem::remove(savePath);
                spdlog::info("No user overrides, removed: {}", savePath);
            }
            return true;
        }
        
        // Write delta to temp file
        std::ofstream file(tempPath);
        if (!file.is_open()) {
            spdlog::error("Failed to open temp file for writing: {}", tempPath);
            return false;
        }
        
        file << userDelta.dump(2); // Pretty print with 2 space indent
        file.close();
        
        // Atomic rename
        if (rename(tempPath.c_str(), savePath.c_str()) != 0) {
            spdlog::error("Failed to rename temp file to: {}", savePath);
            unlink(tempPath.c_str());
            return false;
        }
        
        spdlog::info("Config saved to: {} ({} user-modified top-level keys)", savePath, userDelta.size());
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error saving config: {}", e.what());
        unlink(tempPath.c_str());
        return false;
    }
}

std::string GetAll() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_initialized) {
        return "{}";
    }
    return g_config_tree.dump(2);
}

// Template instantiations
template std::string Get<std::string>(const std::string&, const std::string&);
template int Get<int>(const std::string&, const int&);
template bool Get<bool>(const std::string&, const bool&);
template double Get<double>(const std::string&, const double&);
template float Get<float>(const std::string&, const float&);
template unsigned int Get<unsigned int>(const std::string&, const unsigned int&);
template nlohmann::json Get<nlohmann::json>(const std::string&, const nlohmann::json&);

template bool Set<std::string>(const std::string&, const std::string&);
template bool Set<int>(const std::string&, const int&);
template bool Set<bool>(const std::string&, const bool&);
template bool Set<double>(const std::string&, const double&);
template bool Set<float>(const std::string&, const float&);
template bool Set<unsigned int>(const std::string&, const unsigned int&);
template bool Set<nlohmann::json>(const std::string&, const nlohmann::json&);

template std::optional<std::string> GetOptional<std::string>(const std::string&);
template std::optional<int> GetOptional<int>(const std::string&);
template std::optional<bool> GetOptional<bool>(const std::string&);
template std::optional<double> GetOptional<double>(const std::string&);
template std::optional<float> GetOptional<float>(const std::string&);
template std::optional<unsigned int> GetOptional<unsigned int>(const std::string&);
template std::optional<nlohmann::json> GetOptional<nlohmann::json>(const std::string&);

} // namespace config
} // namespace ipcam
