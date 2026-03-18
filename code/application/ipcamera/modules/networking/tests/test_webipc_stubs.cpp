#include "ipcam/webipc.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace ipcam {
namespace webipc {

// Minimal stubs that satisfy the linker for test builds.

Context::Context(int client_fd) : fd_(client_fd) {}

bool Context::readInt(int32_t &value) { (void)value; return false; }
bool Context::readString(std::string &value) { (void)value; return false; }

bool Context::writeInt(int32_t value) { (void)value; return false; }
bool Context::writeString(const std::string &value) { (void)value; return false; }

// handler registry stubs
static std::unordered_map<std::string, HandlerCallback> g_handlers;
static std::vector<std::shared_ptr<HandlerProvider>> g_providers;

bool start_webipc_server(const std::string &path) { (void)path; return true; }
void stop_webipc_server() {}

void registerHandler(const std::string& name, HandlerCallback handler) {
    g_handlers[name] = handler;
}

void registerModule(std::shared_ptr<HandlerProvider> provider) {
    if (provider) {
        g_providers.push_back(provider);
        provider->registerHandlers();
    }
}

void register_c_handler(const char *name, CHandler h) { (void)name; (void)h; }

bool read_int_from_fd(int fd, int32_t &out) { (void)fd; (void)out; return false; }
bool write_int_to_fd(int fd, int32_t v) { (void)fd; (void)v; return false; }
bool read_string_from_fd(int fd, std::string &out) { (void)fd; (void)out; return false; }
bool write_string_to_fd(int fd, const std::string &s) { (void)fd; (void)s; return false; }

} // namespace webipc
} // namespace ipcam
