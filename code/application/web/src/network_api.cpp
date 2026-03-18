// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "network_api.h"
#include "common.h"
#include "certificate_utils.h"
#include <time.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <fstream>
#include <sys/stat.h>
namespace rockchip {
namespace cgi {

#define USE_RKIPC

#ifdef USE_RKIPC

  uint8_t     httpState = 1;
  int		httpPort = 80;

  uint8_t     httpsState = 1;
  int		httpsPort = 8000;

  uint8_t     rtspState = 1;
  int		rtspPort = 554;

  uint8_t     onvifState = 1;
  int		onvifPort = 5000;
nlohmann::json time_param_get(int stream_id) {
  nlohmann::json param;
  char *tmp = new char[50];
  
}
nlohmann::json network_get_config(std::string interface) {
  int speed, duplex, autoneg;
  // char *tmp = new char[64];
  char mac[64];
  char dns1[64];
  char dns2[64];
  char method[64];
  char address[64];
  char netmask[64];
  char gateway[64];
  char nic_support[256];
  nlohmann::json config, ipv4, link;

  hon_network_ipv4_get(interface.c_str(), method, address, netmask, gateway);
  // hon_network_get_method(&tmp);
  ipv4.emplace("sV4Address", address);
  ipv4.emplace("sV4Gateway", gateway);
  ipv4.emplace("sV4Method", method);
  // ipv4.emplace("sV4Method_1", tmp);
  ipv4.emplace("sV4Netmask", netmask);

  hon_network_dns_get(dns1, dns2);
  link.emplace("sDNS1", dns1);
  link.emplace("sDNS2", dns2);

  hon_network_get_mac(interface.c_str(), mac);
  link.emplace("sMACAddress", mac);
  link.emplace("sInterface", interface.c_str());

  // if (!interface.compare("wlan0")) 
  // {
  //   link.emplace("iPower", 1); // todo
  // } 
  // else 
  // {
  //   // default eth0 on
  //   link.emplace("iPower", 1);
  //   hon_network_nicspeed_get(interface.c_str(), &speed, &duplex, &autoneg);
  //   link.emplace("iDuplex", duplex);
  //   link.emplace("iNicSpeed", speed);
  //   if (autoneg)
  //     link.emplace("sNicSpeed", "Auto");
  //   else if ((speed == 10) && (duplex == 0))
  //     link.emplace("sNicSpeed", "10baseT/Half");
  //   else if ((speed == 10) && (duplex == 1))
  //     link.emplace("sNicSpeed", "10baseT/Full");
  //   else if ((speed == 100) && (duplex == 0))
  //     link.emplace("sNicSpeed", "100baseT/Half");
  //   else if ((speed == 100) && (duplex == 1))
  //     link.emplace("sNicSpeed", "100baseT/Full");
  //   else if ((speed == 1000) && (duplex == 1))
  //     link.emplace("sNicSpeed", "1000baseT/Full");
  //   hon_network_nicspeed_support_get(interface.c_str(), nic_support);
  //   link.emplace("sNicSpeedSupport", nic_support);
  // }

  config.emplace("ipv4", ipv4);
  config.emplace("link", link);

  // delete[] tmp;

  return config;
}


nlohmann::json network_get_ipv4_config(std::string interface) 
{
  char mac[64];
  char method[64];
  char address[64];
  char netmask[64];
  char gateway[64];

  nlohmann::json config, ipv4, link;

  hon_network_ipv4_get(interface.c_str(), method, address, netmask, gateway);
  ipv4.emplace("sV4Address", address);
  ipv4.emplace("sV4Gateway", gateway);
  ipv4.emplace("sV4Method", method);
  ipv4.emplace("sV4Netmask", netmask);

  hon_network_get_mac(interface.c_str(), mac);
  link.emplace("sMACAddress", mac);
  link.emplace("sInterface", interface.c_str());

  config.emplace("ipv4", ipv4);
  config.emplace("link", link);

  return config;
}

nlohmann::json network_get_dns_config() 
{
  char *method = new char[64];
  char dns1[64];
  char dns2[64];

  nlohmann::json config, dns;

  // sprintf(method, "manual");

  hon_network_dns_get(dns1, dns2);

  if(0 == isValidIp4(dns1))
  {
    sprintf(dns1,"255.255.255.255");
  }

  if(0 == isValidIp4(dns2))
  {
      sprintf(dns2,"255.255.255.255");
  }


  dns.emplace("sDNS1", dns1);
  dns.emplace("sDNS2", dns2);

  hon_network_get_dns_method(&method);

  dns.emplace("sDNSMethod", method);

  config.emplace("dns", dns);

  delete[] method;

  return config;
}

nlohmann::json network_get_ports_config() 
{
  nlohmann::json config, state, network_ports;

  // hon_network_ports_get(network_ports);

  hon_network_get_http_port(&httpPort);
  hon_network_get_https_port(&httpsPort);
  hon_network_get_rtsp_port(&rtspPort); 
  hon_network_get_onvif_port(&onvifPort);

  state.emplace("shttpState", httpState);
  network_ports.emplace("shttpPort", httpPort);

  state.emplace("shttpsState", httpsState);
  network_ports.emplace("shttpsPort", httpsPort);

  state.emplace("srtspState", rtspState);
  network_ports.emplace("srtspPort", rtspPort);

  state.emplace("sonvifState", onvifState);
  network_ports.emplace("sonvifPort", onvifPort);

  config.emplace("portState", state);
  config.emplace("networkPorts", network_ports);

  return config;
}

nlohmann::json wifi_power_get() {
  nlohmann::json config;
  int on;
  hon_wifi_power_get(&on);
  config.emplace("iPower", on);
  config.emplace("id", 1);
  config.emplace("sType", "wifi");

  return config;
}

nlohmann::json wifi_list_get() {
  nlohmann::json config;
  char *wifi_list = NULL;
  hon_wifi_get_list(&wifi_list);
  config = nlohmann::json::parse(wifi_list);
  free(wifi_list);

  return config;
}



nlohmann::json ntp_param_get() {
    nlohmann::json param;

    char* timezone  = new char[100];
    int ntp_enable = 0;
    int ntp_refresh_time = 0;
    char* ntp_server  = new char[100];
    int ntp_port = 0;
    int onvif_time_sync_en = 0;

    if (hon_network_get_timezone(&timezone) != 0) {
        return param; // Return blank param for error
    }
    param.emplace("sTimezone", timezone);

    if (hon_network_get_ntp_enable(&ntp_enable) != 0) {
        return param; // Return blank param for error
    }
    param.emplace("iNtpMode", ntp_enable);

    if (hon_network_get_ntp_refresh_time_sec(&ntp_refresh_time) != 0) {
        return param; // Return blank param for error
    }
    param.emplace("iNtpRefreshTime", ntp_refresh_time);

    if (hon_network_get_ntp_server(&ntp_server) != 0) {
        return param; // Return blank param for error
    }
    param.emplace("sNtpServer", ntp_server);

    if (hon_network_get_ntp_port(&ntp_port) != 0) {
        return param; // Return blank param for error
    }
    param.emplace("iNtpPort", ntp_port);

    if (hon_network_get_onvif_timesync_flag(&onvif_time_sync_en) != 0) {
        return param; // Return blank param for error
    }
    param.emplace("iOnvifTimeSyncEn", onvif_time_sync_en);
    param.emplace("sDateTime", current_time_get());
    return param;
}

int ntp_set_param(nlohmann::json param) {
  int value_int;
  std::string value;

  if (param.dump().find("sTimezone") != param.dump().npos) {
    value = param.at("sTimezone").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_network_set_timezone(value.c_str());
  }
  if (param.dump().find("iNtpMode") != param.dump().npos) {
    value_int = atoi(param.at("iNtpMode").dump().c_str());
    hon_network_set_ntp_enable(value_int);
  }
  if (param.dump().find("iNtpRefreshTime") != param.dump().npos) {
    value_int = atoi(param.at("iNtpRefreshTime").dump().c_str());
    hon_network_set_ntp_refresh_time_sec(value_int);
  }
   if (param.dump().find("sNtpServer") != param.dump().npos) {
    value = param.at("sNtpServer").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_network_set_ntp_server(value.c_str());
  }
   if (param.dump().find("iNtpPort") != param.dump().npos) {
     value_int = atoi(param.at("iNtpPort").dump().c_str());
    hon_network_set_ntp_port(value_int);
  }
   if (param.dump().find("iOnvifTimeSyncEn") != param.dump().npos) {
    value_int = atoi(param.at("iOnvifTimeSyncEn").dump().c_str());
    hon_network_set_onvif_timesync_flag(value_int);
  }
  return 0;
}



nlohmann::json network_get_smtp_config() 
{
  nlohmann::json config;
  char *smtp_config = (char *)malloc(2048);
  if (smtp_config == NULL) {
    config = nlohmann::json::object();
    return config;
  }
  
  if (hon_network_smtp_get(&smtp_config) == 0 && smtp_config != NULL) {
    try {
      config = nlohmann::json::parse(smtp_config);
    } catch (nlohmann::json::parse_error& e) {
      config = nlohmann::json::object();
    }
  } else {
    config = nlohmann::json::object();
  }
  
  free(smtp_config);
  return config;
}

void NetworkApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  char *str;
  char pService[255];
  nlohmann::json content;
  std::string path_api_resource;
  std::string path_specific_resource;


  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") 
  {
    if (!path_specific_resource.compare("ipv4")) 
    {
      content = network_get_ipv4_config("eth0");
    }
    else if (!path_specific_resource.compare("dns")) 
    {
      content = network_get_dns_config();
    }
    else if (!path_specific_resource.compare("ports")) 
    {
      content = network_get_ports_config();
    }
    else if (!path_specific_resource.compare("smtp"))
    {
      content = network_get_smtp_config();
      // logger.log(Logger::INFO,"\n======smtp_config===\n", content);
    }

    // if (!path_specific_resource.compare("lan")) {
    //   content = network_get_config("eth0");
    // } else if (!path_specific_resource.compare("wlan")) {
    //   content = network_get_config("wlan0");
    // } else if (!path_specific_resource.compare("wifi")) {
    //   content = wifi_power_get();
    // } else if (!path_specific_resource.compare("wifi-list")) {
    //   content = wifi_list_get();
    // } else if (!path_specific_resource.compare("auto-cnt-wifi")) {
    //   // str = dbserver_sql((char *)"select sService from NetworkService WHERE
    //   // (iAutoconnect = 1) LIMIT 1",
    //   //                      (char *)DBSERVER_NET_INTERFACE);
    //   // content = nlohmann::json::parse(str).at("jData");
    // }
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } 
  else if ((Req.Method == "POST") || (Req.Method == "PUT")) 
  {
  
    nlohmann::json cfg_new = Req.PostObject; // must be json::object
    std::string interface;

    if (path_specific_resource.find("ipv4") != std::string::npos) 
    {
      interface = "eth0";
      nlohmann::json cfg_old = network_get_ipv4_config(interface);
      nlohmann::json diff = nlohmann::json::diff(cfg_old, cfg_new);

      if (diff.dump().find("ipv4") != diff.dump().npos) 
      {
        nlohmann::json cfg_ipv4 = cfg_new.at("ipv4");
        std::string Method = cfg_ipv4.at("sV4Method").dump();
        Method.erase(0, 1).erase(Method.end() - 1, Method.end());     // erase \"
        if (Method == "static") 
        {
          std::string Address = cfg_ipv4.at("sV4Address").dump();
          Address.erase(0, 1).erase(Address.end() - 1, Address.end());
          std::string Netmask = cfg_ipv4.at("sV4Netmask").dump();
          Netmask.erase(0, 1).erase(Netmask.end() - 1, Netmask.end());
          std::string Gateway = cfg_ipv4.at("sV4Gateway").dump();
          Gateway.erase(0, 1).erase(Gateway.end() - 1, Gateway.end());

          content = cfg_new;
          hon_network_ipv4_set((char *)interface.c_str(), (char *)"static",
                              (char *)Address.c_str(), (char *)Netmask.c_str(),
                              (char *)Gateway.c_str());
        } 
        else 
        {
          content = cfg_new;
          hon_network_ipv4_set((char *)interface.c_str(), (char *)"dhcp", NULL,
                              NULL, NULL);
        }
      }
      else
      {
        // Update info
        content = network_get_ipv4_config(interface);
      }     
    }
    else if (path_specific_resource.find("dns") != std::string::npos) 
    {
        nlohmann::json cfg_old = network_get_dns_config();
        nlohmann::json diff = nlohmann::json::diff(cfg_old, cfg_new);

        if (diff.dump().find("dns") != diff.dump().npos) 
        {
          nlohmann::json cfg_dns = cfg_new.at("dns");
          std::string Method = cfg_dns.at("sDNSMethod").dump();
          Method.erase(0, 1).erase(Method.end() - 1, Method.end());   
          if (Method == "manual") 
          {
            std::string dns1 = cfg_dns.at("sDNS1").dump();
            dns1.erase(0, 1).erase(dns1.end() - 1, dns1.end());
            std::string dns2 = cfg_dns.at("sDNS2").dump();
            dns2.erase(0, 1).erase(dns2.end() - 1, dns2.end());

            content = cfg_new;

            // hon_network_set_parameter("network.dns:dns1", (char *)dns1.c_str());
	          // hon_network_set_parameter("network.dns:dns2", (char *)dns2.c_str());

            hon_network_dns_set((char *)dns1.c_str(), (char *)dns2.c_str());

          } 
          else 
          {
            content = cfg_new;
          }
          hon_network_set_dns_method((char *)Method.c_str());
        }
        else
        {
          // Update info
          content = network_get_dns_config();
        } 


    }
    else if (path_specific_resource.find("ports") != std::string::npos) 
    {
        nlohmann::json cfg_old = network_get_ports_config();
        nlohmann::json diff = nlohmann::json::diff(cfg_old, cfg_new);

        if (diff.dump().find("networkPorts") != diff.dump().npos) 
        {
            nlohmann::json cfg_ports = cfg_new.at("networkPorts");

            // int httpState = cfg_ports.at("shttpState").dump();
            // httpState.erase(0, 1).erase(httpState.end() - 1, httpState.end());

            // int httpPort = cfg_ports.at("shttpPort").dump();
            // httpPort.erase(0, 1).erase(httpPort.end() - 1, httpPort.end());
    

            // content = cfg_new;

          httpPort = atoi(cfg_ports.at("shttpPort").dump().c_str());
          httpsPort = atoi(cfg_ports.at("shttpsPort").dump().c_str());
          rtspPort = atoi(cfg_ports.at("srtspPort").dump().c_str());
          onvifPort = atoi(cfg_ports.at("sonvifPort").dump().c_str());

          hon_network_set_http_port(httpPort);
          hon_network_set_https_port(httpsPort);
          hon_network_set_rtsp_port(rtspPort); 
          hon_network_set_onvif_port(onvifPort);

          // hon_video_restart();

            // hon_network_dns_set((char *)dns1.c_str(), (char *)dns2.c_str());

          content = network_get_ports_config();

        }
        else
        {
          // Update info
          content = network_get_ports_config();
        } 
    }
    else if (path_specific_resource.find("smtp") != std::string::npos) 
    {
      nlohmann::json smtp_config = cfg_new;
      
      if (!smtp_config.empty()) {
        // Check for is_test in JSON
        bool is_test = smtp_config.count("is_test") && smtp_config["is_test"].is_boolean() && smtp_config["is_test"].get<bool>();
        std::string smtp_json = smtp_config.dump();
        printf("SMTP SET JSON: %s\n", smtp_json.c_str());
        if (is_test) {
          int test_result = hon_network_smtp_set(smtp_json.c_str());
          printf("test smtp result returned: %d\n", test_result);
          if (test_result == 0) {
            //send  response test successfull
            Resp.setHeader(HttpStatus::kOk, "OK");
            Resp.setApiData(HttpStatus::kOk, smtp_config, "SMTP test success");
            return;
          }
          else {
            //send  response test failed
            Resp.setErrorResponse(HttpStatus::kInternalServerError, "SMTP test failed");
            return;
          }
        } else {
          int set_result = hon_network_smtp_set(smtp_json.c_str());
          printf("hon_network_smtp_set returned: %d\n", set_result);
          if (set_result == 0) {
            // Get updated configuration
            content = network_get_smtp_config();
          } else {
            Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to apply SMTP settings");
            return;
          }
        }
      } else {
        // Update info
        content = network_get_smtp_config();
      }
    }
    else if (path_specific_resource.find("hostname") != std::string::npos) 
    {
    }
    else if (path_specific_resource.find("ntp") != std::string::npos) 
    {
    }
    else{}




    // // Set network/wlan or network/lan
    // if (path_specific_resource.find("lan") != std::string::npos) 
    // {
    //   if (!path_specific_resource.compare("lan"))
    //     interface = "eth0";
    //   else if (!path_specific_resource.compare("wlan"))
    //     interface = "wlan0";

    //   nlohmann::json cfg_old = network_get_config(interface);
    //   nlohmann::json diff = nlohmann::json::diff(cfg_old, cfg_new);
    //   // Set according to the differences
    //   // Set DNS1 and DNS2
    //   // if (diff.dump().find("link") != diff.dump().npos) 
    //   // {
    //   //   std::string dns1 = "";
    //   //   std::string dns2 = "";
    //   //   std::string sNicSpeed = cfg_new.at("link").at("sNicSpeed").dump();
    //   //   std::string Method = cfg_new.at("ipv4").at("sV4Method").dump();
    //   //   sNicSpeed.erase(0, 1).erase(sNicSpeed.end() - 1, sNicSpeed.end());
    //   //   Method.erase(0, 1).erase(Method.end() - 1, Method.end()); // erase \"
    //   //   if (Method == "static") 
    //   //   {
    //   //     dns1 = cfg_new.at("link").at("sDNS1").dump();
    //   //     dns2 = cfg_new.at("link").at("sDNS2").dump();
    //   //     dns1.erase(0, 1).erase(dns1.end() - 1, dns1.end());
    //   //     dns2.erase(0, 1).erase(dns2.end() - 1, dns2.end());
    //   //   }
    //   //   // hon_network_dns_set((char *)dns1.c_str(), (char *)dns2.c_str());

    //   //   // if (!sNicSpeed.compare("10baseT/Half"))
    //   //   //   hon_network_nicspeed_set((char *)interface.c_str(), 10, 0, 0);
    //   //   // else if (!sNicSpeed.compare("10baseT/Full"))
    //   //   //   hon_network_nicspeed_set((char *)interface.c_str(), 10, 1, 0);
    //   //   // else if (!sNicSpeed.compare("100baseT/Half"))
    //   //   //   hon_network_nicspeed_set((char *)interface.c_str(), 100, 0, 0);
    //   //   // else if (!sNicSpeed.compare("100baseT/Full"))
    //   //   //   hon_network_nicspeed_set((char *)interface.c_str(), 100, 1, 0);
    //   //   // else if (!sNicSpeed.compare("1000baseT/Full"))
    //   //   //   hon_network_nicspeed_set((char *)interface.c_str(), 1000, 1, 0);
    //   //   // else
    //   //   //   hon_network_nicspeed_set((char *)interface.c_str(), -1, -1, 1);
    //   // }
    //   if (diff.dump().find("ipv4") != diff.dump().npos) 
    //   {
    //     nlohmann::json cfg_ipv4 = cfg_new.at("ipv4");
    //     std::string Method = cfg_ipv4.at("sV4Method").dump();
    //     Method.erase(0, 1).erase(Method.end() - 1, Method.end()); // erase \"
    //     if (Method == "static") 
    //     {
    //       std::string Address = cfg_ipv4.at("sV4Address").dump();
    //       Address.erase(0, 1).erase(Address.end() - 1, Address.end());
    //       std::string Netmask = cfg_ipv4.at("sV4Netmask").dump();
    //       Netmask.erase(0, 1).erase(Netmask.end() - 1, Netmask.end());
    //       std::string Gateway = cfg_ipv4.at("sV4Gateway").dump();
    //       Gateway.erase(0, 1).erase(Gateway.end() - 1, Gateway.end());

    //       // hon_network_set_method("static"); 
    //       content = cfg_new;
    //       hon_network_ipv4_set((char *)interface.c_str(), (char *)"static",
    //                           (char *)Address.c_str(), (char *)Netmask.c_str(),
    //                           (char *)Gateway.c_str());
    //     } 
    //     else 
    //     {
    //       // hon_network_set_method("dhcp"); 
    //       content = cfg_new;
    //       hon_network_ipv4_set((char *)interface.c_str(), (char *)"dhcp", NULL,
    //                           NULL, NULL);
    //     }
    //   }
    //   else{
    //   // Update info
    //   content = network_get_config(interface);
    //   }
    // } 
    // else if (!path_specific_resource.compare("wifi")) 
    // {
    //   // Set wifi power
    //   if (!Req.Params.empty()) {
    //     for (auto p : Req.Params) {
    //       if (p.Key == "power") {
    //         if (p.Value == "on")
    //           hon_wifi_power_set(1);
    //         else
    //           hon_wifi_power_set(0);
    //         content = wifi_power_get();
    //       } else if (p.Key == "scan") {
    //         hon_wifi_scan_wifi();
    //         content = wifi_list_get();
    //       }
    //     }
    //   }
    //   // Set wifi connect
    //   if (!cfg_new.empty()) {
    //     std::string ssid = cfg_new.at("sName").dump();
    //     ssid.erase(0, 1).erase(ssid.end() - 1, ssid.end());
    //     std::string password = cfg_new.at("sPassword").dump();
    //     password.erase(0, 1).erase(password.end() - 1, password.end());
    //     hon_wifi_connect_with_ssid(ssid.c_str(), password.c_str());
    //     content = wifi_list_get();
    //   }
    // }
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"Network settings applied successfully!");
  } 
  else if (Req.Method == "DELETE") {
    if (!path_specific_resource.compare("wifi")) {
      for (auto p : Req.Params) {
        if (p.Key == "service") {
          std::string ssid = p.Value;
          hon_wifi_forget_with_ssid(ssid.c_str());
          Resp.setHeader(HttpStatus::kOk, "OK");
        }
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kBadRequest, "Not Implemented");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_NTP_SERVER_LENGTH 256

// Function to validate timezone
int validate_timezone( const char *timezone) {
    return strcmp(timezone, "Coordinated Universal Time") == 0;
}

// Function to validate enable flag
int validate_enable(int enable) {
    return enable == 0 || enable == 1;
}

// Function to validate refresh time
int validate_refresh_time(int refresh_time) {
    return refresh_time > 0; // Must be positive
}

// Function to validate ntp_server
int validate_ntp_server(const char *ntp_server) {
    if (strlen(ntp_server) == 0 || strlen(ntp_server) > MAX_NTP_SERVER_LENGTH) {
        return 0; // Invalid if empty or too long
    }
    // Additional checks can be implemented as needed
    return 1; // Assume well-formed for simplicity
}

// Function to validate port
int validate_port(int port) {
    return port >= 0 && port <= 65535; // Valid port range check
}

// Function to validate ONVIF time synchronization flag
int validate_onvif_timesync(int onvif_timesync) {
    return onvif_timesync == 0 || onvif_timesync == 1;
}

// Main validation function
int validate_config( nlohmann::json ntp_data) {
    // Extract values from the JSON object
    std::string timezone = ntp_data.value("sTimezone", "Coordinated Universal Time"); // Default to empty if not found
    int enable = ntp_data.value("iNtpMode", 1); // Default to 0
    int refresh_time = ntp_data.value("iNtpRefreshTime", 60); // Default to 0
    std::string ntp_server = ntp_data.value("sNtpServer", "119.28.183.184"); // Default to empty if not found
    int port = ntp_data.value("iNtpPort", 123); // Default to 0
    int onvif_timesync = ntp_data.value("iOnvifTimeSyncEn", 1); // Default to 0

    if (!validate_timezone(timezone.c_str())) {
        printf("Invalid timezone.\\n");
        return 0;
    }
    if (!validate_enable(enable)) {
        printf("Invalid enable flag.\\n");
        return 0;
    }
    if (!validate_refresh_time(refresh_time)) {
        printf("Invalid refresh time. It should be a positive integer.\\n");
        return 0;
    }
    if (!validate_ntp_server(ntp_server.c_str())) {
        printf("Invalid NTP server.\\n");
        return 0;
    }
    if (!validate_port(port)) {
        printf("Invalid port. It must be between 0 and 65535.\\n");
        return 0;
    }
    if (!validate_onvif_timesync(onvif_timesync)) {
        printf("Invalid ONVIF time synchronization flag.\\n");
        return 0;
    }
    
    return 1; // All validations passed
}


void NetworkNTPApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  nlohmann::json content;
  std::string path_api_resource;
  std::string path_specific_resource;
   /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }
  if (Req.Method == "GET") {
    if (!path_specific_resource.compare("time")) {
       content = ntp_param_get();
      if (!content.empty()) 
				{
					Resp.setHeader(HttpStatus::kOk, "OK");
					Resp.setApiData(HttpStatus::kOk,content,"Time setiings retrieved successfully");
					return;
				}
        else
        {
          Resp.setHeader(HttpStatus::kBadRequest, "No Time Settings found");
          Resp.setApiData(HttpStatus::kBadRequest,content,"No Time Settings found");
          return;
        }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
   
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    
    if (!path_specific_resource.compare("time")) {
      nlohmann::json time_config = Req.PostObject; 
       if(time_config.empty())
        {
          //Resp.setErrorResponse(HttpStatus::kBadRequest, "Missing video configuration");
          Resp.setHeader(HttpStatus::kBadRequest, "Missing Time configuration");
          Resp.setApiData(HttpStatus::kBadRequest,content, "Missing Time configuration");
          return;
        }
        else
        {
          if (validate_config(time_config )==0) {
             Resp.setHeader(HttpStatus::kBadRequest, "Invalid Time configuration parameters");
            Resp.setApiData(HttpStatus::kBadRequest,content, "Invalid Time configuration parameters");
          return;
          }
           if(time_config.at("iNtpMode") == 0)
          {
            std::string time_s = time_config.at("sDateTime");
            time_t stamp_time = standard_to_timestamp(time_s);
            struct timeval tv;
            tv.tv_sec = stamp_time;
            tv.tv_usec = 0;
            int ret = settimeofday(&tv, NULL);
            system("hwclock -w -u");
            if (ret) {
              Resp.setHeader(HttpStatus::kUnauthorized, "Set Time Fail");
              Resp.setApiData(HttpStatus::kUnauthorized,"","Set Time Fail");
              return;
            }
          }
          nlohmann::json cfg_old = ntp_param_get();
          nlohmann::json diff = nlohmann::json::diff(cfg_old, time_config);
          for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
            if (diff.dump().find("\"replace\",\"path\":\"/" + x.key()) ==
                diff.dump().npos)
              time_config.erase(x.key());
          }
        
          ntp_set_param(time_config);
          content = ntp_param_get();
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk,content, "Time setings has been modified successfully.");
          return;
        }
    } else {
      Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
				Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
        return;
    }
  } else {
    Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
				Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
        return;
  }
}

void NetworkDDNSApiHandler::handler(const HttpRequest &Req,
                                    HttpResponse &Resp) {
  nlohmann::json content;

  if (Req.Method == "GET") {
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

void NetworkPPPoEApiHandler::handler(const HttpRequest &Req,
                                     HttpResponse &Resp) {
  nlohmann::json content;

  if (Req.Method == "GET") {
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

void NetworkPortApiHandler::handler(const HttpRequest &Req,
                                    HttpResponse &Resp) {
  nlohmann::json content;

  if (Req.Method == "GET") {
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

#else // USE_RKIPC

nlohmann::json network_get_config(std::string interface) {
  char *str = netserver_get_networkip((char *)interface.c_str());
  nlohmann::json config = nlohmann::json::parse(str).at(0);
  std::string dns1 = "";
  std::string dns2 = "";
  std::string duplex_s = "";
  int duplex = config.at("link").at("iDuplex");
  int speed = config.at("link").at("iNicSpeed");
  std::string v4_method = config.at("dbconfig").at("sV4Method");
  std::string sNicSpeed = config.at("dbconfig").at("sNicSpeed");

  config.at("link").erase("sNicSpeed");
  config.at("link").emplace("sNicSpeed", sNicSpeed);
  if (!v4_method.compare("static")) {
    dns1 = config.at("dbconfig").at("sDNS1");
    dns2 = config.at("dbconfig").at("sDNS2");
    config.at("link").erase("sDNS1");
    config.at("link").erase("sDNS2");
    config.at("link").emplace("sDNS1", dns1);
    config.at("link").emplace("sDNS2", dns2);
  }
  config.erase("dbconfig");
  config.at("ipv4").emplace("sV4Method", v4_method);

  return config;
}

void NetworkApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  char *str;
  char pService[255];
  nlohmann::json content;
  std::string path_api_resource;
  std::string path_specific_resource;

  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") {
    if (!path_specific_resource.compare("lan")) {
      content = network_get_config("eth0");
    } else if (!path_specific_resource.compare("wlan")) {
      content = network_get_config("wlan0");
    } else if (!path_specific_resource.compare("wifi")) {
      str = dbserver_wifi_power_get();
      content = nlohmann::json::parse(str).at("jData").at(0);
    } else if (!path_specific_resource.compare("wifi-list")) {
      str = netserver_get_service((char *)"wifi");
      content = nlohmann::json::parse(str);
    } else if (!path_specific_resource.compare("auto-cnt-wifi")) {
      str = dbserver_sql((char *)"select sService from NetworkService WHERE "
                                 "(iAutoconnect = 1) LIMIT 1",
                         (char *)DBSERVER_NET_INTERFACE);
      content = nlohmann::json::parse(str).at("jData");
    }

    if (!content.empty()) {
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk,content,"OK");
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {

    nlohmann::json cfg_new = Req.PostObject; // must be json::object
    std::string interface;
    // Set network/wlan or network/lan
    if (path_specific_resource.find("lan") != std::string::npos) {
      if (!path_specific_resource.compare("lan"))
        interface = "eth0";
      else if (!path_specific_resource.compare("wlan"))
        interface = "wlan0";
      nlohmann::json cfg_old = network_get_config(interface);
      nlohmann::json diff = nlohmann::json::diff(cfg_old, cfg_new);
      // Set according to the differences
      if (diff.dump().find("ipv4") != diff.dump().npos) {
        nlohmann::json cfg_ipv4 = cfg_new.at("ipv4");
        std::string Method = cfg_ipv4.at("sV4Method").dump();
        Method.erase(0, 1).erase(Method.end() - 1, Method.end()); // erase \"
        if (Method == "static") {
          std::string Address = cfg_ipv4.at("sV4Address").dump();
          Address.erase(0, 1).erase(Address.end() - 1, Address.end());
          std::string Netmask = cfg_ipv4.at("sV4Netmask").dump();
          Netmask.erase(0, 1).erase(Netmask.end() - 1, Netmask.end());
          std::string Gateway = cfg_ipv4.at("sV4Gateway").dump();
          Gateway.erase(0, 1).erase(Gateway.end() - 1, Gateway.end());
          dbserver_network_ipv4_set((char *)interface.c_str(), (char *)"static",
                                    (char *)Address.c_str(),
                                    (char *)Netmask.c_str(),
                                    (char *)Gateway.c_str());
        } else {
          dbserver_network_ipv4_set((char *)interface.c_str(), (char *)"dhcp",
                                    NULL, NULL, NULL);
        }
      }
      // Set DNS1 and DNS2
      if (diff.dump().find("link") != diff.dump().npos) {
        std::string dns1 = "";
        std::string dns2 = "";
        std::string sNicSpeed = cfg_new.at("link").at("sNicSpeed").dump();
        std::string Method = cfg_new.at("ipv4").at("sV4Method").dump();
        sNicSpeed.erase(0, 1).erase(sNicSpeed.end() - 1, sNicSpeed.end());
        Method.erase(0, 1).erase(Method.end() - 1, Method.end()); // erase \"
        if (Method == "static") {
          dns1 = cfg_new.at("link").at("sDNS1").dump();
          dns2 = cfg_new.at("link").at("sDNS2").dump();
          dns1.erase(0, 1).erase(dns1.end() - 1, dns1.end());
          dns2.erase(0, 1).erase(dns2.end() - 1, dns2.end());
        }
        dbserver_network_dns_set((char *)interface.c_str(),
                                 (char *)dns1.c_str(), (char *)dns2.c_str());
        dbserver_network_nicspeed_set((char *)interface.c_str(),
                                      (char *)sNicSpeed.c_str());
      }
      // Update info
      content = network_get_config(interface);
    } else if (!path_specific_resource.compare("wifi")) {
      // Set wifi power
      if (!Req.Params.empty()) {
        for (auto p : Req.Params) {
          if (p.Key == "power") {
            if (p.Value == "on")
              dbserver_wifi_power_set(1);
            else
              dbserver_wifi_power_set(0);
            str = dbserver_wifi_power_get();
            content = nlohmann::json::parse(str).at("jData").at(0);
          } else if (p.Key == "scan") {
            netserver_scan_wifi();
            str = netserver_get_service((char *)"wifi");
            content = nlohmann::json::parse(str);
          }
        }
      }
      // Set wifi connect
      if (!cfg_new.empty()) {
        // netserver_scan_wifi();
        str = netserver_get_service((char *)"wifi");
        nlohmann::json wifi_list = nlohmann::json::parse(str);
        std::string service = cfg_new.at("sService").dump();
        service.erase(0, 1).erase(service.end() - 1, service.end());
        std::string password = cfg_new.at("sPassword").dump();
        password.erase(0, 1).erase(password.end() - 1, password.end());
        int favorite = 1; // invalid parameter
        int autoconnect = atoi(cfg_new.at("iAutoconnect").dump().c_str());
        dbserver_network_service_connect_set((char *)service.c_str(),
                                             (char *)password.c_str(),
                                             &favorite, &autoconnect);
        str = netserver_get_config((char *)service.c_str());
        content = nlohmann::json::parse(str);
      }
    }
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if (Req.Method == "DELETE") {
    if (!path_specific_resource.compare("wifi")) {
      for (auto p : Req.Params) {
        if (p.Key == "service") {
          std::string service = p.Value;
          dbserver_network_service_delete((char *)service.c_str());
          Resp.setHeader(HttpStatus::kOk, "OK");
        }
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kBadRequest, "Not Implemented");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

void NetworkNTPApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  char *str;
  std::string path_api_resource;
  std::string path_specific_resource;
  nlohmann::json content;

  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") {
    if (path_specific_resource.empty()) {
      str = dbserver_ntp_get();
      content = nlohmann::json::parse(str).at("jData").at(0);
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk,content,"OK");
    } else if (path_specific_resource.find("zone") != std::string::npos) {
      str = dbserver_zone_get();
      content = nlohmann::json::parse(str).at("jData");
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk,content,"OK");
    } else if (path_specific_resource.find("time") != std::string::npos) {
      content.emplace("time", current_time_get());
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk,content,"OK");
    } else {
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    if (path_specific_resource.empty()) {
      if (!Req.PostObject.empty()) {
        nlohmann::json ntp_config = Req.PostObject;
        std::string service = ntp_config.at("sNtpServers").dump();
        service.erase(0, 1).erase(service.end() - 1, service.end()); // erase \"
        std::string zone = ntp_config.at("sTimeZone").dump();
        zone.erase(0, 1).erase(zone.end() - 1, zone.end());
        std::string zone_file = ntp_config.at("sTimeZoneFile").dump();
        zone_file.erase(0, 1).erase(zone_file.end() - 1, zone_file.end());
        std::string zone_file_dst = ntp_config.at("sTimeZoneFileDst").dump();
        zone_file_dst.erase(0, 1).erase(zone_file_dst.end() - 1,
                                        zone_file_dst.end());
        // automatic daylight saving time
        int auto_dst = atoi(ntp_config.at("iAutoDst").dump().c_str());
        int update_time = atoi(ntp_config.at("iRefreshTime").dump().c_str());
        int auto_mode = atoi(ntp_config.at("iAutoMode").dump().c_str());
        dbserver_ntp_set((char *)service.c_str(), (char *)zone.c_str(),
                         (char *)zone_file.c_str(),
                         (char *)zone_file_dst.c_str(), &auto_dst, &auto_mode,
                         &update_time);
      }
      // Update info
      content = nlohmann::json::parse(dbserver_ntp_get()).at("jData").at(0);
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk,content,"OK");
    } else if (path_specific_resource.find("time") != std::string::npos) {
      nlohmann::json time_config = Req.PostObject;
      std::string time_s = time_config.at("time");
      time_t stamp_time = standard_to_timestamp(time_s);
      int ret = stime(&stamp_time);
      system("hwclock -w -u");
      if (!ret) {
        content.emplace("time", current_time_get());
        Resp.setHeader(HttpStatus::kOk, "OK");
       Resp.setApiData(HttpStatus::kOk,content,"OK");
      } else {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Set Time Fail");
      }
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

void NetworkDDNSApiHandler::handler(const HttpRequest &Req,
                                    HttpResponse &Resp) {
  if (Req.Method == "GET") {
    nlohmann::json content = nlohmann::json::array();
    auto TEST = R"(
      {
        "enable":"true",
        "DDNSType":"HiDDNS",
        "serveraddress":"www.rock-chips.com",
        "hostName":"wzy02",
        "port":""
      }
    )"_json;
    content.push_back(TEST);
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

void NetworkPPPoEApiHandler::handler(const HttpRequest &Req,
                                     HttpResponse &Resp) {
  if (Req.Method == "GET") {
    nlohmann::json content = nlohmann::json::array();
    auto TEST = R"(
      {
        "enable":"true",
        "deviceDynamicIP":"0.0.0.0",
        "userName":"admin",
        "password":"123456",
        "passwordConfirmation":"123456"
      }
    )"_json;
    content.push_back(TEST);
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

void NetworkPortApiHandler::handler(const HttpRequest &Req,
                                    HttpResponse &Resp) {
  if (Req.Method == "GET") {
    nlohmann::json content;
    char *str = dbserver_port_get();
    nlohmann::json port_all_config = nlohmann::json::parse(str).at("jData");
    nlohmann::json port_current_config;
    int pos_first = Req.PathInfo.find_first_of("/");
    int pos_last = Req.PathInfo.find_last_of("/");

    if (pos_first != pos_last) {
      int id =
          atoi(Req.PathInfo.substr(pos_last + 1, Req.PathInfo.size()).c_str());
      port_current_config = port_all_config.at(id);
      content = port_current_config;
    } else {
      content = port_all_config;
    }

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    nlohmann::json content = nlohmann::json::object();
    nlohmann::json port_config = Req.PostObject; // must be json::object
    int id = 0;
    int pos_first = Req.PathInfo.find_first_of("/");
    int pos_last = Req.PathInfo.find_last_of("/");
    if (pos_first != pos_last)
      id = atoi(Req.PathInfo.substr(pos_last + 1, Req.PathInfo.size()).c_str());

    // Erase unchanged data
    char *prev = dbserver_port_get();
    nlohmann::json cfg_old_all = nlohmann::json::parse(prev).at("jData");
    nlohmann::json diff = nlohmann::json::diff(cfg_old_all.at(id), port_config);
    for (auto &x : nlohmann::json::iterator_wrapper(cfg_old_all.at(id))) {
      if (diff.dump().find(x.key()) == diff.dump().npos)
        port_config.erase(x.key());
    }
    // The port protocol cannot be changed
    if (port_config.dump().find("sProtocol") != port_config.dump().npos)
      port_config.erase("sProtocol");

    // Set new config, only one table is transmitted at a time.
    if (!port_config.empty())
      dbserver_port_set((char *)port_config.dump().c_str(), id);

    // Get new info
    char *str = dbserver_port_get();
    nlohmann::json port_all_config = nlohmann::json::parse(str).at("jData");
    content = port_all_config.at(id);
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(HttpStatus::kOk,content,"OK");
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

//********************************************************************
// SMTP API Handler
//********************************************************************

void NetworkSMTPApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  nlohmann::json content;
  std::string path_api_resource;
  std::string path_specific_resource;

  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") {
    if (path_specific_resource.empty() || !path_specific_resource.compare("config")) {
      // Get SMTP configuration
      char *smtp_config = NULL;
      if (hon_smtp_get_config(&smtp_config) == 0 && smtp_config != NULL) {
        try {
          content = nlohmann::json::parse(smtp_config);
          free(smtp_config);
        } catch (nlohmann::json::parse_error& e) {
          content = nlohmann::json::object();
          if (smtp_config) free(smtp_config);
        }
      } else {
        content = nlohmann::json::object();
      }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "SMTP configuration retrieved successfully");
    } else if (!path_specific_resource.compare("test")) {
      // Test SMTP connection
      char *error_msg = NULL;
      int result = hon_smtp_test_connection(&error_msg);
      
      content = nlohmann::json::object();
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "Connection successful" : "Connection failed";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SMTP connection test completed");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Test Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "SMTP connection test failed");
      }
    } else if (!path_specific_resource.compare("enabled")) {
      // Get SMTP enabled status
      int enabled = 0;
      if (hon_smtp_get_enabled(&enabled) == 0) {
        content["enabled"] = enabled;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SMTP status retrieved");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to get SMTP status");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    if (path_specific_resource.empty() || !path_specific_resource.compare("config")) {
      // Set SMTP configuration
      nlohmann::json smtp_config = Req.PostObject;
      
      if (smtp_config.empty()) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Empty configuration");
        return;
      }

      std::string config_str = smtp_config.dump();
      if (hon_smtp_set_config(config_str.c_str()) == 0) {
        // Get updated config
        char *updated_config = NULL;
        if (hon_smtp_get_config(&updated_config) == 0 && updated_config != NULL) {
          try {
            content = nlohmann::json::parse(updated_config);
            free(updated_config);
          } catch (...) {
            content = smtp_config;
          }
        } else {
          content = smtp_config;
        }
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SMTP configuration updated successfully");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to update SMTP configuration");
      }
    } else if (!path_specific_resource.compare("test-email")) {
      // Send test email
      std::string to_email;
      if (Req.PostObject.find("to") != Req.PostObject.end()) {
        to_email = Req.PostObject["to"].get<std::string>();
      } else {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Recipient email required");
        return;
      }

      char *error_msg = NULL;
      int result = hon_smtp_send_test_email(to_email.c_str(), &error_msg);
      
      content = nlohmann::json::object();
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "Test email sent successfully" : "Failed to send test email";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Test email sent");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Send Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to send test email");
      }
    } else if (!path_specific_resource.compare("enabled")) {
      // Set SMTP enabled status
      if (Req.PostObject.find("enabled") != Req.PostObject.end()) {
        int enabled = Req.PostObject["enabled"].get<int>();
        if (hon_smtp_set_enabled(enabled) == 0) {
          content["enabled"] = enabled;
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "SMTP status updated");
        } else {
          Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to update SMTP status");
        }
      } else {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Missing enabled parameter");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

//********************************************************************
// UPnP API Handler
//********************************************************************

void NetworkUPnPApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  nlohmann::json content;
  std::string path_api_resource;
  std::string path_specific_resource;

  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") {
    if (path_specific_resource.empty() || !path_specific_resource.compare("config")) {
      // Get UPnP configuration
      char *upnp_config = NULL;
      if (hon_upnp_get_config(&upnp_config) == 0 && upnp_config != NULL) {
        try {
          content = nlohmann::json::parse(upnp_config);
          free(upnp_config);
        } catch (nlohmann::json::parse_error& e) {
          content = nlohmann::json::object();
          if (upnp_config) free(upnp_config);
        }
      } else {
        content = nlohmann::json::object();
      }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "UPnP configuration retrieved successfully");
    } else if (!path_specific_resource.compare("device-info")) {
      // Get UPnP device information
      char *device_info = NULL;
      if (hon_upnp_get_device_info(&device_info) == 0 && device_info != NULL) {
        try {
          content = nlohmann::json::parse(device_info);
          free(device_info);
        } catch (nlohmann::json::parse_error& e) {
          content = nlohmann::json::object();
          if (device_info) free(device_info);
        }
      } else {
        content = nlohmann::json::object();
      }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "UPnP device info retrieved");
    } else if (!path_specific_resource.compare("port-mappings")) {
      // List port mappings
      char *mappings = NULL;
      if (hon_upnp_list_port_mappings(&mappings) == 0 && mappings != NULL) {
        try {
          content = nlohmann::json::parse(mappings);
          free(mappings);
        } catch (nlohmann::json::parse_error& e) {
          content = nlohmann::json::object();
          if (mappings) free(mappings);
        }
      } else {
        content = nlohmann::json::object();
      }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "Port mappings retrieved");
    } else if (!path_specific_resource.compare("external-ip")) {
      // Get external IP
      char *external_ip = NULL;
      if (hon_upnp_get_external_ip(&external_ip) == 0 && external_ip != NULL) {
        content["external_ip"] = std::string(external_ip);
        free(external_ip);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "External IP retrieved");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to get external IP");
      }
    } else if (!path_specific_resource.compare("enabled")) {
      // Get UPnP enabled status
      int enabled = 0;
      if (hon_upnp_get_enabled(&enabled) == 0) {
        content["enabled"] = enabled;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "UPnP status retrieved");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to get UPnP status");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    if (path_specific_resource.empty() || !path_specific_resource.compare("config")) {
      // Set UPnP configuration
      nlohmann::json upnp_config = Req.PostObject;
      
      if (upnp_config.empty()) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Empty configuration");
        return;
      }

      std::string config_str = upnp_config.dump();
      if (hon_upnp_set_config(config_str.c_str()) == 0) {
        content = upnp_config;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "UPnP configuration updated successfully");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to update UPnP configuration");
      }
    } else if (!path_specific_resource.compare("discover")) {
      // Discover UPnP devices
      int timeout_ms = 2000;
      if (Req.PostObject.find("timeout_ms") != Req.PostObject.end()) {
        timeout_ms = Req.PostObject["timeout_ms"].get<int>();
      }

      int result = hon_upnp_discover(timeout_ms);
      content["success"] = (result == 0);
      content["message"] = (result == 0) ? "Discovery successful" : "Discovery failed";

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "UPnP discovery completed");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Discovery Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "UPnP discovery failed");
      }
    } else if (!path_specific_resource.compare("add-port-mapping")) {
      // Add port mapping
      nlohmann::json mapping = Req.PostObject;
      
      if (mapping.empty()) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Empty mapping");
        return;
      }

      std::string mapping_str = mapping.dump();
      char *error_msg = NULL;
      int result = hon_upnp_add_port_mapping(mapping_str.c_str(), &error_msg);
      
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "Port mapping added" : "Failed to add port mapping";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Port mapping added");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Add Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to add port mapping");
      }
    } else if (!path_specific_resource.compare("add-camera-ports")) {
      // Add camera ports
      int http_port = 80, https_port = 443, rtsp_port = 554, onvif_port = 8080;
      
      if (Req.PostObject.find("http_port") != Req.PostObject.end())
        http_port = Req.PostObject["http_port"].get<int>();
      if (Req.PostObject.find("https_port") != Req.PostObject.end())
        https_port = Req.PostObject["https_port"].get<int>();
      if (Req.PostObject.find("rtsp_port") != Req.PostObject.end())
        rtsp_port = Req.PostObject["rtsp_port"].get<int>();
      if (Req.PostObject.find("onvif_port") != Req.PostObject.end())
        onvif_port = Req.PostObject["onvif_port"].get<int>();

      char *error_msg = NULL;
      int result = hon_upnp_add_camera_ports(http_port, https_port, rtsp_port, onvif_port, &error_msg);
      
      content["success"] = (result >= 0);
      content["mappings_added"] = (result >= 0) ? result : 0;
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result >= 0) ? "Camera ports configured" : "Failed to configure camera ports";
      }

      if (result >= 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Camera ports configured");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Configuration Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to configure camera ports");
      }
    } else if (!path_specific_resource.compare("enabled")) {
      // Set UPnP enabled status
      if (Req.PostObject.find("enabled") != Req.PostObject.end()) {
        int enabled = Req.PostObject["enabled"].get<int>();
        if (hon_upnp_set_enabled(enabled) == 0) {
          content["enabled"] = enabled;
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "UPnP status updated");
        } else {
          Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to update UPnP status");
        }
      } else {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Missing enabled parameter");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } else if (Req.Method == "DELETE") {
    if (!path_specific_resource.compare("delete-port-mapping")) {
      // Delete port mapping
      if (Req.PostObject.find("external_port") == Req.PostObject.end() ||
          Req.PostObject.find("protocol") == Req.PostObject.end()) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Missing external_port or protocol");
        return;
      }

      int external_port = Req.PostObject["external_port"].get<int>();
      std::string protocol = Req.PostObject["protocol"].get<std::string>();
      
      char *error_msg = NULL;
      int result = hon_upnp_delete_port_mapping(external_port, protocol.c_str(), &error_msg);
      
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "Port mapping deleted" : "Failed to delete port mapping";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Port mapping deleted");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Delete Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to delete port mapping");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

//********************************************************************
// SNMP API Handler
//********************************************************************

void NetworkSNMPApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  nlohmann::json content;
  std::string path_api_resource;
  std::string path_specific_resource;

  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") {
    if (path_specific_resource.empty() || !path_specific_resource.compare("config")) {
      // Get SNMP configuration
      char *snmp_config = NULL;
      if (hon_snmp_get_config(&snmp_config) == 0 && snmp_config != NULL) {
        try {
          content = nlohmann::json::parse(snmp_config);
          free(snmp_config);
        } catch (nlohmann::json::parse_error& e) {
          content = nlohmann::json::object();
          if (snmp_config) free(snmp_config);
        }
      } else {
        content = nlohmann::json::object();
      }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "SNMP configuration retrieved successfully");
    } else if (!path_specific_resource.compare("status")) {
      // Get SNMP daemon status
      char *status_json = NULL;
      if (hon_snmp_get_status(&status_json) == 0 && status_json != NULL) {
        try {
          content = nlohmann::json::parse(status_json);
          free(status_json);
        } catch (nlohmann::json::parse_error& e) {
          content = nlohmann::json::object();
          if (status_json) free(status_json);
        }
      } else {
        content = nlohmann::json::object();
      }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "SNMP status retrieved");
    } else if (!path_specific_resource.compare("test")) {
      // Test SNMP configuration
      char *error_msg = NULL;
      int result = hon_snmp_test(&error_msg);
      
      content = nlohmann::json::object();
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "SNMP test successful" : "SNMP test failed";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SNMP test completed");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Test Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "SNMP test failed");
      }
    } else if (!path_specific_resource.compare("enabled")) {
      // Get SNMP enabled status
      int enabled = 0;
      if (hon_snmp_get_enabled(&enabled) == 0) {
        content["enabled"] = enabled;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SNMP status retrieved");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to get SNMP status");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    if (path_specific_resource.empty() || !path_specific_resource.compare("config")) {
      // Set SNMP configuration
      nlohmann::json snmp_config = Req.PostObject;
      
      if (snmp_config.empty()) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Empty configuration");
        return;
      }

      std::string config_str = snmp_config.dump();
      if (hon_snmp_set_config(config_str.c_str()) == 0) {
        // Get updated config
        char *updated_config = NULL;
        if (hon_snmp_get_config(&updated_config) == 0 && updated_config != NULL) {
          try {
            content = nlohmann::json::parse(updated_config);
            free(updated_config);
          } catch (...) {
            content = snmp_config;
          }
        } else {
          content = snmp_config;
        }
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SNMP configuration updated successfully");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to update SNMP configuration");
      }
    } else if (!path_specific_resource.compare("start")) {
      // Start SNMP daemon
      char *error_msg = NULL;
      int result = hon_snmp_start(&error_msg);
      
      content = nlohmann::json::object();
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "SNMP daemon started" : "Failed to start SNMP daemon";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SNMP daemon started");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Start Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to start SNMP daemon");
      }
    } else if (!path_specific_resource.compare("stop")) {
      // Stop SNMP daemon
      char *error_msg = NULL;
      int result = hon_snmp_stop(&error_msg);
      
      content = nlohmann::json::object();
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "SNMP daemon stopped" : "Failed to stop SNMP daemon";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SNMP daemon stopped");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Stop Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to stop SNMP daemon");
      }
    } else if (!path_specific_resource.compare("restart")) {
      // Restart SNMP daemon
      char *error_msg = NULL;
      int result = hon_snmp_restart(&error_msg);
      
      content = nlohmann::json::object();
      content["success"] = (result == 0);
      if (error_msg) {
        content["message"] = std::string(error_msg);
        free(error_msg);
      } else {
        content["message"] = (result == 0) ? "SNMP daemon restarted" : "Failed to restart SNMP daemon";
      }

      if (result == 0) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "SNMP daemon restarted");
      } else {
        Resp.setHeader(HttpStatus::kBadRequest, "Restart Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to restart SNMP daemon");
      }
    } else if (!path_specific_resource.compare("enabled")) {
      // Set SNMP enabled status
      if (Req.PostObject.find("enabled") != Req.PostObject.end()) {
        int enabled = Req.PostObject["enabled"].get<int>();
        if (hon_snmp_set_enabled(enabled) == 0) {
          content["enabled"] = enabled;
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "SNMP status updated");
        } else {
          Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to update SNMP status");
        }
      } else {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Missing enabled parameter");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

//********************************************************************
// HTTPS/SSL Certificate API Handler
//********************************************************************

void NetworkHTTPSApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  nlohmann::json content;
  std::string path_api_resource;
  std::string path_specific_resource;

  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
  }

  if (Req.Method == "GET") {
    if (path_specific_resource.empty() || !path_specific_resource.compare("config")) {
      // Get HTTPS configuration
      content = nlohmann::json::object();
      content["certificate_type"] = get_current_cert_type();
      content["https_enabled"] = true; // HTTPS is always enabled
      content["https_port"] = httpsPort;
      content["force_https"] = false;

      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "HTTPS configuration retrieved successfully");
    } 
    else if (!path_specific_resource.compare("certificate-info")) {
      // Get certificate information
      std::string cert_type = get_current_cert_type();
      const char *cert_path = (cert_type == "custom") ? CUSTOM_CERT_PATH : DEFAULT_CERT_PATH;

      CertificateInfo cert_info;
      cert_info.type = cert_type;

      if (get_certificate_info(cert_path, cert_info) == 0) {
        content = cert_info_to_json(cert_info);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Certificate information retrieved successfully");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to read certificate information");
      }
    }
    else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  } 
  else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    if (!path_specific_resource.compare("config")) {
      // Update HTTPS configuration
      nlohmann::json https_config = Req.PostObject;
      
      if (https_config.empty()) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Empty configuration");
        return;
      }

      // Update certificate type if specified
      if (https_config.find("certificate_type") != https_config.end()) {
        std::string cert_type = https_config["certificate_type"].get<std::string>();
        if (set_cert_type(cert_type) == 0) {
          // Update symlinks based on type
          unlink("/etc/nginx/ssl/current.crt");
          unlink("/etc/nginx/ssl/current.key");
          
          if (cert_type == "custom") {
            symlink(CUSTOM_CERT_PATH, "/etc/nginx/ssl/current.crt");
            symlink(CUSTOM_KEY_PATH, "/etc/nginx/ssl/current.key");
          } else {
            symlink(DEFAULT_CERT_PATH, "/etc/nginx/ssl/current.crt");
            symlink(DEFAULT_KEY_PATH, "/etc/nginx/ssl/current.key");
          }
          reload_nginx();
        }
      }

      // Return updated configuration
      content["certificate_type"] = get_current_cert_type();
      content["https_enabled"] = true;
      content["https_port"] = httpsPort;
      content["force_https"] = false;

      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "HTTPS configuration updated successfully");
    }
    else if (!path_specific_resource.compare("upload-certificate")) {
      // Handle certificate upload
      // Note: File upload handling needs to be implemented in the request parser
      // For now, we'll assume files are saved to TEMP_CERT_PATH and TEMP_KEY_PATH
      
      // Check if temp files exist
      struct stat cert_stat, key_stat;
      if (stat(TEMP_CERT_PATH, &cert_stat) != 0 || stat(TEMP_KEY_PATH, &key_stat) != 0) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Certificate or key file not uploaded");
        return;
      }

      // Validate certificate is not expired
      if (is_cert_expired(TEMP_CERT_PATH) == 1) {
        unlink(TEMP_CERT_PATH);
        unlink(TEMP_KEY_PATH);
        
        content["success"] = false;
        content["error_code"] = "CERT_EXPIRED";
        content["message"] = "Certificate has expired";
        Resp.setHeader(HttpStatus::kBadRequest, "Certificate Expired");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Certificate has expired");
        return;
      }

      // Install the certificate
      if (install_custom_certificate(TEMP_CERT_PATH, TEMP_KEY_PATH) == 0) {
        // Get certificate info
        CertificateInfo cert_info;
        cert_info.type = "custom";
        get_certificate_info(CUSTOM_CERT_PATH, cert_info);

        content["success"] = true;
        content["message"] = "Certificate installed successfully";
        content["data"] = nlohmann::json::object();
        content["data"]["certificate_type"] = "custom";
        content["data"]["subject"] = cert_info.subject;
        content["data"]["valid_to"] = cert_info.valid_to;

        // Clean up temp files
        unlink(TEMP_CERT_PATH);
        unlink(TEMP_KEY_PATH);

        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Certificate installed successfully");
      } else {
        // Clean up temp files
        unlink(TEMP_CERT_PATH);
        unlink(TEMP_KEY_PATH);

        content["success"] = false;
        content["error_code"] = "CERT_MISMATCH";
        content["message"] = "Invalid certificate format or certificate does not match private key";
        Resp.setHeader(HttpStatus::kBadRequest, "Installation Failed");
        Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to install certificate");
      }
    }
    else if (!path_specific_resource.compare("use-default")) {
      // Switch to default certificate
      if (set_cert_type("default") == 0) {
        // Update symlinks
        unlink("/etc/nginx/ssl/current.crt");
        unlink("/etc/nginx/ssl/current.key");
        symlink(DEFAULT_CERT_PATH, "/etc/nginx/ssl/current.crt");
        symlink(DEFAULT_KEY_PATH, "/etc/nginx/ssl/current.key");
        
        reload_nginx();

        content["success"] = true;
        content["message"] = "Switched to default certificate successfully";
        content["data"] = nlohmann::json::object();
        content["data"]["certificate_type"] = "default";

        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Switched to default certificate");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to switch to default certificate");
      }
    }
    else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  }
  else if (Req.Method == "DELETE") {
    if (!path_specific_resource.compare("certificate")) {
      // Delete custom certificate
      if (delete_custom_certificate() == 0) {
        content["success"] = true;
        content["message"] = "Custom certificate deleted successfully. Reverted to default certificate.";

        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "Certificate deleted");
      } else {
        Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to delete certificate");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotFound, "Resource not found");
    }
  }
  else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

#endif

} // namespace cgi
} // namespace rockchip
