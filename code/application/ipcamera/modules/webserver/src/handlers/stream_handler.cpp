#include "stream_handler.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleStream(const api::RequestContext& ctx) {
    (void)ctx;  // Unused for this endpoint
    
    json j;
    j["streams"] = {
        {
            {"id", "main"},
            {"resolution", "1920x1080"},
            {"fps", 30},
            {"codec", "h264"},
            {"url", "rtsp://camera.local/main"}
        },
        {
            {"id", "sub"},
            {"resolution", "640x480"},
            {"fps", 15},
            {"codec", "h264"},
            {"url", "rtsp://camera.local/sub"}
        }
    };
    
    api::Response resp;
    resp.status_code = 200;
    resp.body = j.dump(2);
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
