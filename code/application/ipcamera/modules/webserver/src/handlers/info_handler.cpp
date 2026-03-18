#include "info_handler.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleInfo(const api::RequestContext& ctx) {
    (void)ctx;  // Unused for this endpoint
    
    json j;
    j["device"] = {
        {"model", "IP Camera"},
        {"firmware", "1.0.0"},
        {"manufacturer", "IPCAM"}
    };
    j["capabilities"] = {
        {"video", true},
        {"audio", true},
        {"ptz", false},
        {"ai", true}
    };
    
    api::Response resp;
    resp.status_code = 200;
    resp.body = j.dump(2);
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
