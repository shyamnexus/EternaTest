#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <yaml-cpp/yaml.h>
#include <mutex>

namespace ipcam {
namespace config {

class ConfigLoader {
public:
  static ConfigLoader& Instance();

  bool LoadDirectory(const std::string& dir);
  bool LoadFiles(const std::vector<std::string>& files);

  template<typename T>
  T Get(const std::string& path, const T& defaultVal) const;

  template<typename T>
  bool Set(const std::string& path, const T& value);

  template<typename T>
  std::optional<T> GetOptional(const std::string& path) const;

  bool Has(const std::string& path) const;
  bool Remove(const std::string& path);
  bool Reload();
  bool Save(const std::string& filePath = "") const;
  bool Validate(std::string &outError) const;

private:
  ConfigLoader();
  YAML::Node merged_;
  std::vector<std::string> sources_;
  mutable std::mutex mutex_;
  std::string last_dir_;

  void deepMerge(YAML::Node &dst, const YAML::Node &src);
  YAML::Node getNode(const std::string& path) const;
  bool setNode(const std::string& path, const YAML::Node& value);
  
  // Validation helper methods
  bool validateNetworkConfig(std::string &outError) const;
  bool validateStorageConfig(std::string &outError) const;
  bool validateMediaConfig(std::string &outError) const;
  bool validateSecurityConfig(std::string &outError) const;
};

} // namespace config
} // namespace ipcam

// Template implementation for Get
template<typename T>
T ipcam::config::ConfigLoader::Get(const std::string& path, const T& defaultVal) const {
  YAML::Node n = getNode(path);
  if(!n || n.IsNull()) return defaultVal;
  try {
    return n.as<T>();
  } catch(...) {
    return defaultVal;
  }
}

// Template implementation for GetOptional
template<typename T>
std::optional<T> ipcam::config::ConfigLoader::GetOptional(const std::string& path) const {
  YAML::Node n = getNode(path);
  if(!n || n.IsNull()) return std::nullopt;
  try {
    return n.as<T>();
  } catch(...) {
    return std::nullopt;
  }
}

// Template implementation for Set
template<typename T>
bool ipcam::config::ConfigLoader::Set(const std::string& path, const T& value) {
  std::lock_guard<std::mutex> lock(mutex_);
  try {
    YAML::Node valueNode;
    valueNode = value;
    return setNode(path, valueNode);
  } catch(...) {
    return false;
  }
}
