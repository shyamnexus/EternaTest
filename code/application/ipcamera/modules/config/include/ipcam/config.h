#pragma once
#include <string>
#include <optional>

namespace ipcam {
namespace config {

inline const char* config_info() { return "config"; }

// Initialize the config subsystem. On success returns true; on failure returns false and
// writes a human-readable error message to `outError`.
bool Init(const std::string &configDir, std::string &outError);

// Shutdown the config subsystem and release any resources. Safe to call multiple times.
void Shutdown();

// Standardized configuration accessor functions
template<typename T>
T Get(const std::string& path, const T& defaultValue);

template<typename T>
bool Set(const std::string& path, const T& value);

template<typename T>
std::optional<T> GetOptional(const std::string& path);

bool Has(const std::string& path);
bool Remove(const std::string& path);
bool Save(const std::string& filePath = "");

// Get the entire configuration tree as a JSON string
std::string GetAll();

} // namespace config
} // namespace ipcam
