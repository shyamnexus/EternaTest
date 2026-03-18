#include "config_handler.h"
#include <ipcam/config.h>
#include <nlohmann/json.hpp>
#include <functional>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleConfig(const api::RequestContext& ctx) {
    api::Response resp;
    
    if (ctx.method == "GET") {
        // Return current configuration
        resp.status_code = 200;
        resp.body = config::GetAll();
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        // Update configuration
        try {
            auto j = json::parse(ctx.body);
            
            // Merge incoming changes into config
            for (auto it = j.begin(); it != j.end(); ++it) {
                std::string key = it.key();
                if (it.value().is_object()) {
                    // Nested object - need to iterate
                    std::function<void(const json&, const std::string&)> processObject;
                    processObject = [&processObject](const json& obj, const std::string& prefix) {
                        for (auto nested = obj.begin(); nested != obj.end(); ++nested) {
                            std::string path = prefix.empty() ? nested.key() : prefix + "." + nested.key();
                            if (nested.value().is_object()) {
                                processObject(nested.value(), path);
                            } else if (nested.value().is_string()) {
                                config::Set<std::string>(path, nested.value());
                            } else if (nested.value().is_number_integer()) {
                                config::Set<int>(path, nested.value());
                            } else if (nested.value().is_boolean()) {
                                config::Set<bool>(path, nested.value());
                            } else if (nested.value().is_number_float()) {
                                config::Set<double>(path, nested.value());
                            }
                        }
                    };
                    processObject(it.value(), key);
                } else if (it.value().is_string()) {
                    config::Set<std::string>(key, it.value());
                } else if (it.value().is_number_integer()) {
                    config::Set<int>(key, it.value());
                } else if (it.value().is_boolean()) {
                    config::Set<bool>(key, it.value());
                } else if (it.value().is_number_float()) {
                    config::Set<double>(key, it.value());
                }
            }
            
            // Save the updated config
            if (config::Save()) {
                resp.status_code = 200;
                json result;
                result["status"] = "success";
                result["message"] = "Configuration updated and saved";
                resp.body = result.dump(2);
            } else {
                resp.status_code = 500;
                json err;
                err["error"] = "Failed to save configuration";
                resp.body = err.dump(2);
            }
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json err;
            err["error"] = e.what();
            resp.body = err.dump(2);
        }
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
