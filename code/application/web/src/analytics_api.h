// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __CGI_ANALYTICS_API_H__
#define __CGI_ANALYTICS_API_H__

#include "api_handler.h"
#include <unistd.h>
#include <regex>
#define INFO_BUFFER_LENGTH 1024
#define BUFFER_LENGTH 256
#define SHORT_INFO_BUFFER_LENGTH 64
#define LONG_INFO_BUFFER_LENGTH 5000
#define MAX_RULE_COUNT 6

#define trespass "tt:TrespassingDetectionRuleEngine"
#define tripwire "tt:TripWireDetectionRuleEngine"
#define motion "tt:MotionDetectionRuleEngine"
#define tamper "tt:CameraTamperingDetectionRuleEngine"


#define trespass_module "TrespassingDetectionModule"
#define tripwire_module "TripWireDetectionModule"
#define motion_module "MotionDetectionModule"
#define tamper_module "TamperDetectionModule"




namespace rockchip {
namespace cgi {

class AnalyticsApiHandler : public ApiHandler {
public:
  AnalyticsApiHandler();
  ~AnalyticsApiHandler() = default;

  virtual void handler(const HttpRequest &Req, HttpResponse &Resp) final;
  RetCode getModules(const HttpRequest &Req);
  RetCode getRules(const HttpRequest &Req);
  RetCode updateModule(const HttpRequest &Req, nlohmann::json &content);
  RetCode updateRule(const HttpRequest &Req, nlohmann::json &content);
  RetCode createRule(const HttpRequest &Req, nlohmann::json &content);
  RetCode getRuleOptions(const HttpRequest &Req, nlohmann::json &content);
  RetCode getModuleOptions(const HttpRequest &Req, nlohmann::json &content);
  RetCode getAllRules(const HttpRequest &Req, nlohmann::json &content);
  RetCode getAlerts(const HttpRequest &Req, nlohmann::json &content);
  RetCode deleteRules(const HttpRequest &Req, nlohmann::json &content);
  RetCode getRuleByName(const HttpRequest &Req, nlohmann::json &content);
  RetCode getCameraTamperingOptions(const HttpRequest &Req, nlohmann::json &content);
  RetCode setCameraTamperingOptions(const HttpRequest &Req, nlohmann::json &content);

private:
  std::map<std::string, std::string> camera_tamper_type;
  bool check_coordinates_format(const std::string& coordinates);
  bool check_line_intersection(std::pair<int, int>& p11, std::pair<int, int>& p12, std::pair<int, int>& p21, std::pair<int, int>& p22);
  bool check_zone_convexity(std::string coordinates);
  int get_coordinate_size(std::string coordinates);
  std::vector<std::pair<int,int>> parse_coordinates(const std::string& coordinates);
};

} // namespace cgi
} // namespace rockchip

#endif // __CGI_ANALYTICS_API_H__
