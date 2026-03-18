// Minimal test for networking module C++ API
#include <cstdio>
#include <iostream>
#include <string>

#include "ipcam/network_manager.h"

using namespace ipcam::networking;

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    bool ok = true;
    NetworkManager& nm = NetworkManager::Instance();

    // Test IPv4 validation
    if (!nm.ValidateIpv4Address("127.0.0.1")) {
        std::printf("ValidateIpv4Address(127.0.0.1) failed\n");
        ok = false;
    }
    if (nm.ValidateIpv4Address("not.an.ip")) {
        std::printf("ValidateIpv4Address(not.an.ip) unexpectedly true\n");
        ok = false;
    }

    // Test DNS get
    auto dns_result = nm.GetDns();
    if (!dns_result.success) {
        std::printf("GetDns failed: %s\n", dns_result.error.c_str());
    } else {
        std::printf("DNS1: %s\n", dns_result.value.primary.c_str());
        if (!dns_result.value.secondary.empty()) {
            std::printf("DNS2: %s\n", dns_result.value.secondary.c_str());
        }
    }

    // Test MAC retrieval for eth0
    auto mac_result = nm.GetMac("eth0");
    if (mac_result.success) {
        std::printf("eth0 MAC: %s\n", mac_result.value.c_str());
    } else {
        std::printf("GetMac(eth0) failed: %s\n", mac_result.error.c_str());
    }

    // Test IPv4 get for eth0
    auto ipv4_result = nm.GetIpv4("eth0");
    if (ipv4_result.success) {
        std::printf("eth0: method=%s address=%s netmask=%s gateway=%s\n",
                    ipv4_result.value.method.c_str(),
                    ipv4_result.value.address.c_str(),
                    ipv4_result.value.netmask.c_str(),
                    ipv4_result.value.gateway.c_str());
    } else {
        std::printf("GetIpv4(eth0) failed: %s\n", ipv4_result.error.c_str());
    }

    // Test port configuration
    auto http_port = nm.GetHttpPort();
    if (http_port.success) {
        std::printf("HTTP Port: %d\n", http_port.value);
    }

    std::printf(ok ? "TESTS PASS\n" : "TESTS FAIL\n");
    return ok ? 0 : 1;
}
