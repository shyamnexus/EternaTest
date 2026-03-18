#include "ipcam/config_loader.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

int main(){
  auto &cfg = ipcam::config::ConfigLoader::Instance();
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
    std::cout << "No configs directory found in candidates" << std::endl;
    return 1;
  }

  std::cout << "Using configs directory: " << chosen << std::endl;

  // enumerate YAML files
  std::vector<std::string> files;
  for(auto &p: fs::directory_iterator(chosen)){
    if(p.is_regular_file()){
      auto ext = p.path().extension().string();
      if(ext == ".yaml" || ext == ".yml") files.push_back(p.path().string());
    }
  }
  std::sort(files.begin(), files.end());

  if(files.empty()){
    std::cout << "No yaml files found in " << chosen << std::endl;
    return 1;
  }

  // Print each YAML file parsed
  for(auto &f: files){
    std::cout << "---- " << f << " ----" << std::endl;
    try{
      YAML::Node n = YAML::LoadFile(f);
      YAML::Emitter out;
      out << n;
      std::cout << out.c_str() << std::endl;
    }catch(const std::exception &e){
      std::cout << "Failed to parse " << f << ": " << e.what() << std::endl;
      // fallback: print raw file
      std::ifstream ifs(f);
      std::string line;
      while(std::getline(ifs, line)) std::cout << line << std::endl;
    }
  }

  // Load all files via ConfigLoader and show merged result
  if(!cfg.LoadFiles(files)){
    std::cout << "ConfigLoader::LoadFiles failed" << std::endl;
    return 1;
  }

  int port = cfg.Get<int>("network.http.port", 80);
  std::cout << "Merged network.http.port = " << port << std::endl;
  return 0;
}
