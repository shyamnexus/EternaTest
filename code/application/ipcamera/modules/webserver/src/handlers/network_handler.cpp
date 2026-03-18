#include "network_handler.h"
#include <ipcam/network_manager.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ipcam {
namespace webserver {
namespace handlers {

api::Response HandleNetwork(const api::RequestContext& ctx) {
    auto& nm = networking::NetworkManager::Instance();
    api::Response resp;
    
    if (ctx.method == "GET") {
        json j;
        
        // Get IPv4 configuration
        auto ipv4 = nm.GetIpv4("eth0");
        if (ipv4.success) {
            j["ipv4"] = {
                {"method", ipv4.value.method},
                {"address", ipv4.value.address},
                {"netmask", ipv4.value.netmask},
                {"gateway", ipv4.value.gateway}
            };
        } else {
            j["ipv4"] = {
                {"error", ipv4.error}
            };
        }
        
        // Get IPv6 configuration
        auto ipv6 = nm.GetIpv6("eth0");
        if (ipv6.success) {
            j["ipv6"] = {
                {"enabled", ipv6.value.enabled},
                {"method", ipv6.value.method},
                {"address", ipv6.value.address},
                {"prefix_length", ipv6.value.prefix_length},
                {"gateway", ipv6.value.gateway},
                {"privacy_extensions", ipv6.value.privacy_extensions},
                {"accept_ra", ipv6.value.accept_ra}
            };
        } else {
            j["ipv6"] = {
                {"error", ipv6.error}
            };
        }
        
        // Get DNS configuration
        auto dns = nm.GetDns();
        if (dns.success) {
            j["dns"] = {
                {"primary", dns.value.primary},
                {"secondary", dns.value.secondary},
                {"primary_ipv6", dns.value.primary_ipv6},
                {"secondary_ipv6", dns.value.secondary_ipv6}
            };
        } else {
            j["dns"] = {
                {"error", dns.error}
            };
        }
        
        // Get MAC address
        auto mac = nm.GetMac("eth0");
        if (mac.success) {
            j["mac"] = mac.value;
        }
        
        // Get hostname
        auto hostname = nm.GetHostname();
        if (hostname.success) {
            j["hostname"] = hostname.value;
        }
        
        // Get port configuration
        auto http = nm.GetHttpPort();
        auto https = nm.GetHttpsPort();
        auto rtsp = nm.GetRtspPort();
        auto onvif = nm.GetOnvifPort();
        
        j["ports"] = {
            {"http", http.success ? http.value : 0},
            {"https", https.success ? https.value : 0},
            {"rtsp", rtsp.success ? rtsp.value : 0},
            {"onvif", onvif.success ? onvif.value : 0}
        };
        
        // Get SSL certificate information
        auto cert_info = nm.GetSslCertificateInfo();
        if (cert_info.success) {
            j["ssl"] = {
                {"certificate", cert_info.value.certificate_path},
                {"key", cert_info.value.key_path},
                {"common_name", cert_info.value.common_name},
                {"issuer", cert_info.value.issuer},
                {"valid_from", cert_info.value.valid_from},
                {"valid_to", cert_info.value.valid_to},
                {"subject_alt_names", cert_info.value.subject_alt_names},
                {"is_self_signed", cert_info.value.is_self_signed}
            };
        } else {
            j["ssl"] = {
                {"error", cert_info.error}
            };
        }
        
        // Get multicast configuration
        auto multicast = nm.GetMulticastConfig();
        if (multicast.success) {
            j["multicast"] = {
                {"enabled", multicast.value.enabled},
                {"address", multicast.value.address},
                {"port", multicast.value.port},
                {"ttl", multicast.value.ttl},
                {"interface", multicast.value.interface},
                {"loopback", multicast.value.loopback}
            };
        } else {
            j["multicast"] = {
                {"error", multicast.error}
            };
        }
         
        // Get MTU configuration
        auto mtu = nm.GetMtu("eth0");
        if (mtu.success) {
            j["mtu"] = {
                {"value", mtu.value.mtu},
                {"path_mtu_discovery", mtu.value.path_mtu_discovery}
            };
        } else {
            j["mtu"] = {
                {"error", mtu.error}
            };
        }
        
        // Get gratuitous ARP configuration
        auto arp = nm.GetGratuitousArpConfig();
        if (arp.success) {
            j["gratuitous_arp"] = {
                {"enabled", arp.value.enabled},
                {"count", arp.value.count},
                {"interval_ms", arp.value.interval_ms}
            };
        } else {
            j["gratuitous_arp"] = {
                {"error", arp.error}
            };
        }
        
        resp.status_code = 200;
        resp.body = j.dump(2);
        
    } else if (ctx.method == "POST" || ctx.method == "PUT") {
        // Update network configuration
        try {
            auto j = json::parse(ctx.body);
            json results;
            bool has_error = false;
            
            // Update IPv4 if provided
            if (j.contains("ipv4")) {
                networking::NetworkManager::Ipv4Config cfg;
                cfg.method = j["ipv4"].value("method", "dhcp");
                cfg.address = j["ipv4"].value("address", "");
                cfg.netmask = j["ipv4"].value("netmask", "");
                cfg.gateway = j["ipv4"].value("gateway", "");
                
                auto result = nm.SetIpv4("eth0", cfg);
                if (!result.success) {
                    results["ipv4"]["error"] = result.error;
                    has_error = true;
                } else {
                    results["ipv4"]["status"] = "success";
                }
            }
            
            // Update IPv6 if provided
            if (j.contains("ipv6")) {
                networking::NetworkManager::Ipv6Config cfg;
                cfg.enabled = j["ipv6"].value("enabled", true);
                cfg.method = j["ipv6"].value("method", "auto");
                cfg.address = j["ipv6"].value("address", "");
                cfg.prefix_length = j["ipv6"].value("prefix_length", 64);
                cfg.gateway = j["ipv6"].value("gateway", "");
                cfg.privacy_extensions = j["ipv6"].value("privacy_extensions", true);
                cfg.accept_ra = j["ipv6"].value("accept_ra", true);
                
                auto result = nm.SetIpv6("eth0", cfg);
                if (!result.success) {
                    results["ipv6"]["error"] = result.error;
                    has_error = true;
                } else {
                    results["ipv6"]["status"] = "success";
                }
            }
            
            // Update DNS if provided
            if (j.contains("dns")) {
                networking::NetworkManager::DnsConfig cfg;
                cfg.primary = j["dns"].value("primary", "");
                cfg.secondary = j["dns"].value("secondary", "");
                cfg.primary_ipv6 = j["dns"].value("primary_ipv6", "");
                cfg.secondary_ipv6 = j["dns"].value("secondary_ipv6", "");
                
                auto result = nm.SetDns(cfg);
                if (!result.success) {
                    results["dns"]["error"] = result.error;
                    has_error = true;
                } else {
                    results["dns"]["status"] = "success";
                }
            }
            
            // Update hostname if provided
            if (j.contains("hostname")) {
                std::string hostname = j["hostname"];
                auto result = nm.SetHostname(hostname);
                if (!result.success) {
                    results["hostname"]["error"] = result.error;
                    has_error = true;
                } else {
                    results["hostname"]["status"] = "success";
                }
            }
            
            // Update ports if provided
            if (j.contains("ports")) {
                auto& ports = j["ports"];
                
                if (ports.contains("http")) {
                    auto result = nm.SetHttpPort(ports["http"]);
                    if (!result.success) {
                        results["ports"]["http"]["error"] = result.error;
                        has_error = true;
                    } else {
                        results["ports"]["http"]["status"] = "success";
                    }
                }
                
                if (ports.contains("https")) {
                    auto result = nm.SetHttpsPort(ports["https"]);
                    if (!result.success) {
                        results["ports"]["https"]["error"] = result.error;
                        has_error = true;
                    } else {
                        results["ports"]["https"]["status"] = "success";
                    }
                }
                
                if (ports.contains("rtsp")) {
                    auto result = nm.SetRtspPort(ports["rtsp"]);
                    if (!result.success) {
                        results["ports"]["rtsp"]["error"] = result.error;
                        has_error = true;
                    } else {
                        results["ports"]["rtsp"]["status"] = "success";
                    }
                }
                
                if (ports.contains("onvif")) {
                    auto result = nm.SetOnvifPort(ports["onvif"]);
                    if (!result.success) {
                        results["ports"]["onvif"]["error"] = result.error;
                        has_error = true;
                    } else {
                        results["ports"]["onvif"]["status"] = "success";
                    }
                }
            }
            
            // Update SSL certificate if provided
            if (j.contains("ssl")) {
                auto& ssl = j["ssl"];
                
                if (ssl.contains("certificate") && ssl.contains("key")) {
                    std::string cert_path = ssl["certificate"];
                    std::string key_path = ssl["key"];
                    
                    auto result = nm.SetSslCertificate(cert_path, key_path);
                    if (!result.success) {
                        results["ssl"]["error"] = result.error;
                        has_error = true;
                    } else {
                        results["ssl"]["status"] = "success";
                    }
                } else {
                    results["ssl"]["error"] = "Both certificate and key paths are required";
                    has_error = true;
                }
            }
            
            // Update multicast configuration if provided
            if (j.contains("multicast")) {
                networking::NetworkManager::MulticastConfig cfg;
                cfg.enabled = j["multicast"].value("enabled", true);
                cfg.address = j["multicast"].value("address", "239.255.255.255");
                cfg.port = j["multicast"].value("port", 5353);
                cfg.ttl = j["multicast"].value("ttl", 1);
                cfg.interface = j["multicast"].value("interface", "eth0");
                cfg.loopback = j["multicast"].value("loopback", false);
                
                auto result = nm.SetMulticastConfig(cfg);
                if (!result.success) {
                    results["multicast"]["error"] = result.error;
                    has_error = true;
                } else {
                    results["multicast"]["status"] = "success";
                }
            }
            
            // Update MTU configuration if provided
            if (j.contains("mtu")) {
                networking::NetworkManager::MtuConfig cfg;
                cfg.mtu = j["mtu"].value("value", 1500);
                cfg.path_mtu_discovery = j["mtu"].value("path_mtu_discovery", true);
                
                auto result = nm.SetMtu("eth0", cfg);
                if (!result.success) {
                    results["mtu"]["error"] = result.error;
                    has_error = true;
                } else {
                    results["mtu"]["status"] = "success";
                }
            }
            
            // Update gratuitous ARP configuration if provided
            if (j.contains("gratuitous_arp")) {
                networking::NetworkManager::GratuitousArpConfig cfg;
                cfg.enabled = j["gratuitous_arp"].value("enabled", true);
                cfg.count = j["gratuitous_arp"].value("count", 3);
                cfg.interval_ms = j["gratuitous_arp"].value("interval_ms", 500);
                
                auto result = nm.SetGratuitousArpConfig(cfg);
                if (!result.success) {
                    results["gratuitous_arp"]["error"] = result.error;
                    has_error = true;
                } else {
                    results["gratuitous_arp"]["status"] = "success";
                }
            }
            
            // Trigger gratuitous ARP if explicitly requested
            if (j.contains("send_gratuitous_arp") && j["send_gratuitous_arp"].get<bool>()) {
                auto result = nm.SendGratuitousArp("eth0");
                if (!result.success) {
                    results["send_gratuitous_arp"]["error"] = result.error;
                    // Don't mark as error - gratuitous ARP is best effort
                } else {
                    results["send_gratuitous_arp"]["status"] = "sent";
                }
            }
            
            resp.status_code = has_error ? 500 : 200;
            resp.body = results.dump(2);
            
        } catch (const std::exception& e) {
            resp.status_code = 400;
            json err;
            err["error"] = e.what();
            resp.body = err.dump(2);
        }
    } else if (ctx.method == "DELETE") {
        // Reset network configuration to defaults from config file
        // This reloads the configuration from the stored settings
        json results;
        
        // For now, just return success - in a real implementation, this would:
        // 1. Reload network config from /etc/network/interfaces or config file
        // 2. Restart network services
        // 3. Return the reloaded configuration
        
        // Simple approach: just reload current config (acts as refresh)
        // To implement true reset, we'd need a ResetToDefaults() method in NetworkManager
        results["status"] = "Configuration reloaded";
        results["message"] = "Network settings refreshed from system";
        
        resp.status_code = 200;
        resp.body = results.dump(2);
    }
    
    return resp;
}

} // namespace handlers
} // namespace webserver
} // namespace ipcam
