#include "status_handler.h"
#include <nlohmann/json.hpp>
#include <ctime>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleStatus(const api::RequestContext& ctx) {
    (void)ctx;  // Unused for this endpoint
    
    json j;
    j["status"] = "ok";
    j["version"] = "1.0.0";
    j["timestamp"] = std::time(nullptr);
    
    api::Response resp;
    resp.status_code = 200;
    resp.body = j.dump(2);
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
