// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "analytics_api.h"
#include "common.h"
#include "Logger.h"
namespace rockchip {
namespace cgi {

AnalyticsApiHandler::AnalyticsApiHandler() {
  camera_tamper_type["image_too_bright"] = "Image Too Bright";
  camera_tamper_type["image_too_dark"] = "Image Too Dark";
  camera_tamper_type["image_too_blurry"] = "Image Too Blurry";
  camera_tamper_type["global_scene_change"] = "Global Scene Change";
}




void AnalyticsApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
    HttpResponseHandler RespHandler;
	nlohmann::json content = nlohmann::json({});
    std::string path_api_resource;
    std::string path_specific_resource;
    std::string path_channel_resource;
    char *str;
    int ret = 0;
	RetCode ret_code;
	std::string resp_message;
	// Get Path Information
	int pos_first = Req.PathInfo.find_first_of("/");
	path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
	pos_first = path_api_resource.find_first_of("/");
	if (pos_first != -1) {
		path_specific_resource = path_api_resource.substr(pos_first + 1, path_api_resource.size());
		pos_first = path_specific_resource.find_first_of("/");
		if (pos_first != -1) {
			path_channel_resource = path_specific_resource.substr( pos_first + 1, path_specific_resource.size());
		}
	}

	if (Req.Method == "GET") {
		if (path_specific_resource.find("getmodules") != std::string::npos) 
		{
			char *tmp = new char[LONG_INFO_BUFFER_LENGTH];
			if (tmp == NULL) {
				Resp.setHeader(HttpStatus::kInternalServerError, "Server Request failed");
				getStringFromErrorCode(RetCode::API_MEM_ALLOC_FAILURE, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Server Request failed in fetching modules");
				// Resp.setErrorResponse(HttpStatus::kInternalServerError, "Server Request failed");
				return;
			}
			char sql_query[] = "SELECT ID, name AS ModuleName, active AS isActive FROM analytics_modules;";
			
			ret = hon_analytics_find_records(tmp,sql_query);
			if (ret!=0)
			{	
				if (tmp) {
					delete[] tmp;
				}
				Resp.setHeader(HttpStatus::kInternalServerError, "Server Request failed");
				getStringFromErrorCode(RetCode::API_SQL_DB_FAILURE, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Server Request failed in fetching modules");
				// Resp.setErrorResponse(HttpStatus::kInternalServerError, "Server Request failed");
				return;
			}

			content = nlohmann::json::parse(tmp);
			nlohmann::json analytics_modules = nlohmann::json({});
			analytics_modules["Result"] = nlohmann::json::array();
			
			for (int i = 0; i<content["ModuleName"].size();i++) {
			nlohmann::json dump_module;
			dump_module["ID"] = content["ID"].at(i);
			dump_module["Name"] = content["ModuleName"].at(i);
			dump_module["isActive"] = content["isActive"].at(i);
			analytics_modules["Result"].push_back(dump_module);
			}

			
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, analytics_modules, "OK");
			if (tmp) {
				delete[] tmp;
			}
		}	
    	else if(path_specific_resource.find("getrules") != std::string::npos) 
		{
			if (Req.Params.empty()) {
		        // Resp.setErrorResponse(HttpStatus::kBadRequest, "No parameter defined");
				Resp.setHeader(HttpStatus::kBadRequest, "No parameter defined");
				Resp.setApiData(HttpStatus::kBadRequest, content, "No parameter defined");
      		}
			RetCode request_check = RetCode::API_REQUEST_CHECK_FAILURE;
			// Logger& logger = Logger::getInstance();
			for (auto p : Req.Params) {
				if (p.Key == "moduleName") {
					str = (char*)malloc(SHORT_INFO_BUFFER_LENGTH);
					if (str == NULL) {
						Resp.setHeader(HttpStatus::kInternalServerError, "Server Request failed in fetching rules");
						getStringFromErrorCode(RetCode::API_MEM_ALLOC_FAILURE, resp_message);
						Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
						// Resp.setApiData(HttpStatus::kInternalServerError, content, "Server Request failed in fetching rules");
						return;
					}
					// logger.log(Logger::INFO,"\n============comes in loop for params============\n");
					convert_space_in_param(p.Value);
					strncpy(str, p.Value.c_str(), SHORT_INFO_BUFFER_LENGTH - 1);

					str[SHORT_INFO_BUFFER_LENGTH - 1] = '\0';
					request_check = RetCode::API_REQUEST_CHECK_SUCCESS;

				}
			}
			if (request_check == RetCode::API_REQUEST_CHECK_FAILURE) {
				Resp.setHeader(HttpStatus::kBadRequest, "Invalid parameter defined");
				getStringFromErrorCode(RetCode::API_REQUEST_CHECK_FAILURE, resp_message);
				Resp.setApiData(HttpStatus::kBadRequest, content, resp_message);
				// Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid parameter defined");
				return;
			}
			char *tmp = new char[LONG_INFO_BUFFER_LENGTH];
			if (tmp == NULL) {
				Resp.setHeader(HttpStatus::kInternalServerError, "Server Request failed in fetching rules");
				getStringFromErrorCode(RetCode::API_MEM_ALLOC_FAILURE, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Server Request failed in fetching rules");
				return;
			}
			// logger.log(Logger::INFO,"\n============comes in analytics_api.cpp rule_list============\n");
			char sql_query[INFO_BUFFER_LENGTH] = {0};
			snprintf(sql_query,INFO_BUFFER_LENGTH,"select ID, name as RuleName, active as isActive, label as Label, coordinates as Coordinates from analytics_rules where module_name = '%s';",str);
		
			// logger.log(Logger::INFO,"\n============comes in analytics_api.pp rule_list============ ",sql_query);
			ret = hon_analytics_find_records(tmp, sql_query);
			if (ret!=0)
			{	
				if (tmp) {
					delete[] tmp;
				}
				if (str) {
					free(str);
				}
				Resp.setHeader(HttpStatus::kInternalServerError, "Server Request failed in fetching rules");
				getStringFromErrorCode(RetCode::API_SQL_DB_FAILURE, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Server Request failed in fetching rules");
				return;
			}

			content = nlohmann::json::parse(tmp);
			nlohmann::json analytics_rules = nlohmann::json({});
			analytics_rules["Result"] = nlohmann::json::array();
			
			for (int i = 0; i<content["RuleName"].size();i++) {
				nlohmann::json dump_rule;
				dump_rule["ID"] = content["ID"].at(i);
				dump_rule["Name"] = content["RuleName"].at(i);
				dump_rule["isActive"] = content["isActive"].at(i);
				dump_rule["Label"] = content["Label"].at(i);
				dump_rule["Coordinates"] = content["Coordinates"].at(i);
				analytics_rules["Result"].push_back(dump_rule);
			}
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, analytics_rules, "OK");
			if (tmp) {
					delete[] tmp;
				}
			if (str) {
				free(str);
			}

		}
		else if(path_specific_resource.find("getalerts") != std::string::npos) {
	
			ret_code = getAlerts(Req, content);
			if (ret_code != RetCode::API_SUCCESS){
				Resp.setHeader(HttpStatus::kBadRequest, "Get Alerts Request failed");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kBadRequest, content, resp_message);
				// Resp.setApiData(HttpStatus::kBadRequest, content, "Get Alerts Request failed with error code "+std::to_string(static_cast<int>(ret_code)));
				return;
			}
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, content, "OK");

		}		
		else if(path_specific_resource.find("getallrules") != std::string::npos) {
	
			ret_code = getAllRules(Req, content);
			if (ret_code != RetCode::API_SUCCESS){
				Resp.setHeader(HttpStatus::kInternalServerError, "Get All Rules Request failed with errorcode");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Get All Rules Request failed with errorcode "+std::to_string(static_cast<int>(ret_code)));
				return;
			}
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, content, "OK");

		}

		else if(path_specific_resource.find("getrulebyname") != std::string::npos) {
			
			if (Req.Params.empty()) {
				Resp.setHeader(HttpStatus::kBadRequest, "Get Rule by name Request invalid argument");
				getStringFromErrorCode(RetCode::API_INAVLID_ARGUMENT, resp_message);
				Resp.setApiData(HttpStatus::kBadRequest, content, resp_message);
				// Resp.setApiData(HttpStatus::kBadRequest, content, "Get Rule by name Request invalid argument");
				return;
				
			}

			ret_code = getRuleByName(Req, content);
			if (ret_code != RetCode::API_SUCCESS){
				Resp.setHeader(HttpStatus::kInternalServerError, "Get Rule by name Request failed with errorcode");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Get Rule by name Request failed with errorcode "+std::to_string(static_cast<int>(ret_code)));
				return;
			}
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, content, "OK");

		}
		else if(path_specific_resource.find("getruleoptions") != std::string::npos) { 
			if (Req.Params.empty()) {
				Resp.setHeader(HttpStatus::kBadRequest, "Get All Rules Request invalid argument");
				getStringFromErrorCode(RetCode::API_INAVLID_ARGUMENT, resp_message);
				Resp.setApiData(HttpStatus::kBadRequest, content, resp_message);
				// Resp.setApiData(HttpStatus::kBadRequest, content, "Get All Rules Request invalid argument");
				return;
			}

			ret_code = getRuleOptions(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setHeader(HttpStatus::kInternalServerError, "Get All Rules Request failed with errorcode");
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Get All Rules Request failed with errorcode "+std::to_string(static_cast<int>(ret_code)));
				return;
			}
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, content, "OK");
		}
		else if(path_specific_resource.find("getmoduleoptions") != std::string::npos) { 
			ret_code = getModuleOptions(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{
				Resp.setHeader(HttpStatus::kInternalServerError, "Get Module Options Request failed with errorcode");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Get Module Options Request failed with errorcode "+std::to_string(static_cast<int>(ret_code)));
				return;
			}	
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, content, "OK");
		}
		else if(path_specific_resource.find("getcameratamperingoptions") != std::string::npos) { 
			ret_code = getCameraTamperingOptions(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{
				Resp.setHeader(HttpStatus::kInternalServerError, "Get Camera Tampering Options failed.");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				return;
			}	
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk, content, "OK");
		}
		else {
			Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
			getStringFromErrorCode(RetCode::API_NOT_IMPLEMENTED, resp_message);
			Resp.setApiData(HttpStatus::kNotImplemented, content, resp_message);
			// Resp.setApiData(HttpStatus::kNotImplemented, content, "Not Implemented. error code:"+std::to_string(static_cast<int>(RetCode::API_FAILURE)));
			return;
		}

	} 
	else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    
		if(path_specific_resource.find("updatemodule") != std::string::npos) {
			ret_code  = updateModule(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setHeader(HttpStatus::kInternalServerError, "Module update Request failed");
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// RespHandler.setResponse(Resp, HttpStatus::kInternalServerError, ret_code, "Module update Request failed");
				return;
			}

		} 
		else if (path_specific_resource.find("createrule") != std::string::npos) {
			ret_code = createRule(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{	
				Resp.setHeader(HttpStatus::kInternalServerError, "Rule creation Request failed");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Rule creation Request failed with errorcode:"+std::to_string(static_cast<int>(ret_code)));
				return;
			} 
		}
		else if (path_specific_resource.find("deleterule") != std::string::npos) {
			ret_code = deleteRules(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{
				Resp.setHeader(HttpStatus::kInternalServerError, "Rule Delete Request failed");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Rule Delete Request failed with errorcode:"+std::to_string(static_cast<int>(ret_code)));
				return;
			}
		}
		else if (path_specific_resource.find("updaterule") != std::string::npos) {
			ret_code = updateRule(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{
				Resp.setHeader(HttpStatus::kInternalServerError, "Rule update Request failed");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kInternalServerError, content, resp_message);
				// Resp.setApiData(HttpStatus::kInternalServerError, content, "Rule update Request failed with errorcode:"+std::to_string(static_cast<int>(ret_code)));
				return;
			}
    	}
		else if (path_specific_resource.find("setcameratamperingoptions") != std::string::npos) {
			ret_code = setCameraTamperingOptions(Req, content);
			if (ret_code != RetCode::API_SUCCESS)
			{
				Resp.setHeader(HttpStatus::kBadRequest, "Set Camera Tampering Request invalid argument");
				getStringFromErrorCode(ret_code, resp_message);
				Resp.setApiData(HttpStatus::kBadRequest, content, resp_message);
				// Resp.setApiData(HttpStatus::kBadRequest, content, "Set Camera Tampering Request failed with Errorcode:"+std::to_string(static_cast<int>(ret_code)));
				return;
			}
		}
		else {
		
			Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
			getStringFromErrorCode(RetCode::API_NOT_IMPLEMENTED, resp_message);
			Resp.setApiData(HttpStatus::kNotImplemented, content, resp_message);
			// Resp.setApiData(HttpStatus::kNotImplemented, content, "Not Implemented. Errorcode:"+std::to_string(static_cast<int>(RetCode::API_FAILURE)));
			return;
		}

		content["Result"] = "Record updated successfully.";
		Resp.setHeader(HttpStatus::kOk, "OK");
		Resp.setApiData(HttpStatus::kOk, content, "OK");
	}	 
	else {
			Resp.setHeader(HttpStatus::kInternalServerError, "Not Implemented");
			getStringFromErrorCode(RetCode::API_NOT_IMPLEMENTED, resp_message);
			Resp.setApiData(HttpStatus::kNotImplemented, content, resp_message);
			// Resp.setApiData(HttpStatus::kInternalServerError, content, "Not Implemented. Errorcode:"+std::to_string(static_cast<int>(RetCode::API_FAILURE)));
  	}
}

RetCode AnalyticsApiHandler::getAllRules(const HttpRequest &Req, nlohmann::json &content) {
	char* tmp;
	try {
		int ret = 1;
		tmp = new char[LONG_INFO_BUFFER_LENGTH];    
		if (tmp == NULL) {
			return RetCode::API_MEM_ALLOC_FAILURE;
		} 
		char sql_query[INFO_BUFFER_LENGTH] = {0};
		// memset(sql_query, '\0', 1);
		snprintf(sql_query, INFO_BUFFER_LENGTH, "SELECT ID AS ID, name AS RuleName, active AS isActive, label AS Label, behaviour AS DetectionMode, sensitivity AS Sensitivity, alarm_enable_colour AS AlarmOnColour, alarm_disable_colour AS AlarmOffColour FROM analytics_rules;");
		// char sql_query[INFO_BUFFER_LENGTH] = ";
		
		sql_query[strlen(sql_query)] = '\0';
		ret = hon_analytics_find_records(tmp, sql_query);
		if (ret != 0)
		{
			if(tmp) {
				delete[] tmp;
			}
			return RetCode::API_SQL_DB_FAILURE;
		}
		nlohmann::json  analytics_rules = nlohmann::json::parse(tmp);
		content = nlohmann::json({});
		content["Result"] = nlohmann::json::array();
		
		for (int i = 0; i<analytics_rules["RuleName"].size();i++) {
			nlohmann::json dump_rule;
			dump_rule["ID"] = analytics_rules["ID"].at(i);
			dump_rule["Name"] = analytics_rules["RuleName"].at(i);
			dump_rule["isActive"] = analytics_rules["isActive"].at(i);
			dump_rule["Label"] = analytics_rules["Label"].at(i);
			dump_rule["DetectionMode"] = analytics_rules["DetectionMode"].at(i);
			dump_rule["Sensitivity"] = analytics_rules["Sensitivity"].at(i);
			dump_rule["AlarmOnColour"] = analytics_rules["AlarmOnColour"].at(i);
			dump_rule["AlarmOffColour"] = analytics_rules["AlarmOffColour"].at(i);
			content["Result"].push_back(dump_rule);
		}
		if (tmp){ 
			delete[] tmp;
		}
		return RetCode::API_SUCCESS;

	}
	catch (const std::exception &e) {
		if(tmp) {
			delete[] tmp;
		}
		return RetCode::API_FAILURE;
	}

}


RetCode AnalyticsApiHandler::getRuleByName(const HttpRequest &Req, nlohmann::json &content) {
	char* rule_name;
	char* tmp;
	try {

		RetCode request_check = RetCode::API_REQUEST_CHECK_FAILURE;
		for (auto p : Req.Params) {
			if (p.Key == "ruleName") {
				rule_name = (char*)malloc(SHORT_INFO_BUFFER_LENGTH);
				if (rule_name == NULL) {
					return RetCode::API_MEM_ALLOC_FAILURE;
				}
				convert_space_in_param(p.Value);
				strcpy(rule_name, p.Value.c_str());
				rule_name[strlen(rule_name)] = '\0';
				request_check = RetCode::API_REQUEST_CHECK_SUCCESS;
			}
		}

		if (request_check == RetCode::API_REQUEST_CHECK_FAILURE) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		int ret = 1;
		tmp = new char[LONG_INFO_BUFFER_LENGTH];
		if (tmp == NULL) {
			return RetCode::API_MEM_ALLOC_FAILURE;
		}      
		char sql_query[INFO_BUFFER_LENGTH] = {0};
		memset(sql_query, '\0', 1);
		snprintf(sql_query, INFO_BUFFER_LENGTH, "SELECT ar.*, at.common_type FROM analytics_rules ar JOIN analytics_type at ON ar.type = at.rule_type WHERE ar.name = '%s';", rule_name);		
		sql_query[strlen(sql_query)] = '\0';
		ret = hon_analytics_find_records(tmp, sql_query);
		if (ret != 0)
		{	
			if (tmp) {
				delete[] tmp;
			}
			if (rule_name) {
				free(rule_name);
			}
			return RetCode::API_SQL_DB_FAILURE;
		}
		nlohmann::json  analytics_rules = nlohmann::json::parse(tmp);
		content = nlohmann::json({});
		// content["Result"] = nlohmann::json::array();
		
		for (int i = 0; i<analytics_rules["ID"].size();i++) {
			// nlohmann::json dump_rule;
			content["ID"] = analytics_rules["ID"].at(i);
			content["Name"] = analytics_rules["name"].at(i);
			content["isActive"] = analytics_rules["active"].at(i);
			content["Label"] = analytics_rules["label"].at(i);
			content["DetectionMode"] = analytics_rules["behaviour"].at(i);
			content["Sensitivity"] = analytics_rules["sensitivity"].at(i);
			content["Coordinates"] = analytics_rules["coordinates"].at(i);
			content["AlarmOnColour"] = analytics_rules["alarm_enable_colour"].at(i);
			content["InvasionTime"] = analytics_rules["invasion_time"].at(i);
			content["AlarmOffColour"] = analytics_rules["alarm_disable_colour"].at(i);
			content["ModuleName"] = analytics_rules["common_type"].at(i);
			// content["Result"].push_back(dump_rule);
			break;
		}
		if (tmp) {
			delete[] tmp;
		}
		if (rule_name) {
			free(rule_name);
		}
		return RetCode::API_SUCCESS;

	}
	catch (const std::exception &e) {
		if (tmp) {
			delete[] tmp;
		}
		if (rule_name) {
			free(rule_name);
		}
		return RetCode::API_FAILURE;
	}

}



RetCode AnalyticsApiHandler::getAlerts(const HttpRequest &Req, nlohmann::json &content) {
	char * tmp;
	try {
		int ret = 1;
		tmp = new char[LONG_INFO_BUFFER_LENGTH];
		if (tmp == NULL) {
			return RetCode::API_MEM_ALLOC_FAILURE;
		}  
		char sql_query[INFO_BUFFER_LENGTH] = {0};
		// memset(sql_query, '\0', 1);
		snprintf(sql_query,INFO_BUFFER_LENGTH, "SELECT * FROM alert ORDER BY alert_time DESC LIMIT 20;");
		// char sql_query[INFO_BUFFER_LENGTH] = ";
		
		// logger.log(Logger::INFO,"\n============comes in analytics_api.pp rule_list============ ",sql_query);
		sql_query[strlen(sql_query)] = '\0';
		ret = hon_analytics_find_records(tmp, sql_query);
		if (ret != 0)
		{	
			if (tmp) {
				delete[] tmp;
			}
			return RetCode::API_SQL_DB_FAILURE;
		}
		// logger.log(Logger::INFO,"\n============tmp============\n", tmp);
		nlohmann::json  alerts = nlohmann::json::parse(tmp);
		content = nlohmann::json({});
		content["Result"] = nlohmann::json::array();
		
		for (int i = 0; i<alerts["ID"].size();i++) {
			nlohmann::json dump_alert;
			dump_alert["ID"] = alerts["ID"].at(i);
			dump_alert["RuleName"] = alerts["rule_name"].at(i);
			dump_alert["Behaviour"] = alerts["behaviour"].at(i);
			dump_alert["Label"] = alerts["label"].at(i);
			dump_alert["Alert Time"] = alerts["alert_time"].at(i);
			dump_alert["Alert Type"] = alerts["alert_type"].at(i);
			content["Result"].push_back(dump_alert);
		}
		if (tmp) {
				delete[] tmp;
			}
		return RetCode::API_SUCCESS;

	}
	catch (const std::exception &e) {
		if (tmp) {
				delete[] tmp;
			}
		return RetCode::API_FAILURE;
	}

}

RetCode AnalyticsApiHandler::updateModule(const HttpRequest &Req, nlohmann::json &content) {
	try {
		int ret = 1;
		int active = 0;
		// Logger& logger = Logger::getInstance();
		nlohmann::json post_body = Req.PostObject;
		if (post_body.empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if (post_body["ModuleName"].empty() || post_body["isActive"].empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if (post_body["isActive"].get<std::string>() == "true") {
			active = 1;
		}

		char sql_query[INFO_BUFFER_LENGTH] = {0};
		// memset(sql_query, '\0', 1);
		snprintf(sql_query, INFO_BUFFER_LENGTH, "UPDATE analytics_modules SET active = %d where name = '%s';", active, post_body["ModuleName"].get<std::string>().c_str());
		sql_query[strlen(sql_query)] = '\0';
		ret = hon_analytics_update_db(sql_query);
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
			ret = hon_update_analytics_module(); //restart the analytics module for the changes to take effect 
			return RetCode::API_SUCCESS;
	}
	catch (const std::exception &e) {
		return RetCode::API_FAILURE;
	}
}


RetCode AnalyticsApiHandler::updateRule(const HttpRequest &Req, nlohmann::json &content) {
	try {

		int ret = 1;
		int active = 0;
		int rule_count = 0;
		int coord_size = 0;
		bool zone_convextity = false;
		const char *rule_type;
		const char *module_name;
		const char *coordinates;
		const char *canvas_size;
		const char *label;
		float sensitivity = 0.0;
		float invasion_time = 0.0;
		const char *behaviour;
		const char *alarm_enable_colour;
		const char *alarm_disable_colour;
		std::string coordinates_str;
		std::string canvas_size_str;
		std::string label_str;
		std::string behaviour_str;
		std::string alarm_enable_colour_str;
		std::string alarm_disable_colour_str;
		bool check_coord;
		// Logger& logger = Logger::getInstance();
		nlohmann::json post_body = Req.PostObject;
		if (post_body.empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if (post_body["ID"].empty() || post_body["RuleName"].empty() || post_body["isActive"].empty() || post_body["RuleType"].empty() || post_body["CanvasSize"].empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		//regex to check if rulename contains special characters but space
		std::regex special_characters("[^a-zA-Z0-9_ -]");
		if (std::regex_search(post_body["RuleName"].get<std::string>(), special_characters)) {
			return RetCode::INVALID_RULE_NAME;
		}

		//regex to check if rulename starts with a number
		std::regex starts_with_number("^[0-9]");
		if (std::regex_search(post_body["RuleName"].get<std::string>(), starts_with_number)) {
			return RetCode::INVALID_RULE_NAME;
		}
		

		if (post_body["RuleType"].get<std::string>() =="Trespass" ) {
			if (post_body["Coordinates"].empty() || post_body["Label"].empty() || post_body["Sensitivity"].empty() || post_body["InvasionTime"].empty() || post_body["DetectionMode"].empty() || post_body["AlarmOnColour"].empty() || post_body["AlarmOffColour"].empty()) {
				return RetCode::API_INAVLID_ARGUMENT;
			}
			rule_type = trespass;
			module_name = trespass_module;
			// strcpy(coordinates,(char*)post_body["Coordinates"].get<std::string>().c_str());
			coordinates_str = post_body["Coordinates"].get<std::string>();
			check_coord = this->check_coordinates_format(coordinates_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			// check if coordinates are in correct size
			coord_size = get_coordinate_size(coordinates_str);
			if (coord_size < 3 || coord_size > 8) {
				return RetCode::TRESPASS_COORDINATE_SIZE_FAILURE;
			}
			// check if coordinates are convex
			zone_convextity =  check_zone_convexity(coordinates_str);
			if (zone_convextity == true) {
				return RetCode::NON_CONVEX_COORDINATES;
			}
			coordinates = coordinates_str.c_str();
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			check_coord = this->check_coordinates_format(canvas_size_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			canvas_size = canvas_size_str.c_str();


			label_str = post_body["Label"].get<std::string>();
			label = label_str.c_str();
			sensitivity = post_body["Sensitivity"].get<float>();
			invasion_time = post_body["InvasionTime"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
			
		}
		else if (post_body["RuleType"].get<std::string>() =="Tripwire" ) {
			if (post_body["Coordinates"].empty() || post_body["Label"].empty() || post_body["Sensitivity"].empty() || post_body["DetectionMode"].empty() || post_body["AlarmOnColour"].empty() || post_body["AlarmOffColour"].empty()) {
				return RetCode::API_INAVLID_ARGUMENT;
			}
			rule_type = tripwire;
			module_name = tripwire_module;
			coordinates_str = post_body["Coordinates"].get<std::string>();
			check_coord = this->check_coordinates_format(coordinates_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			// check if coordinates are in correct size
			coord_size = get_coordinate_size(coordinates_str);
			if (coord_size != 2) {
				return RetCode::TRIPWIRE_COORDINATE_SIZE_FAILURE;
			}

			coordinates = coordinates_str.c_str();
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			check_coord = this->check_coordinates_format(canvas_size_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			canvas_size = canvas_size_str.c_str();
			label_str = post_body["Label"].get<std::string>();
			label = label_str.c_str();
			sensitivity = post_body["Sensitivity"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
		}
		else if (post_body["RuleType"].get<std::string>() =="Motion" ) {
			if (post_body["Label"].empty() || post_body["Sensitivity"].empty() || post_body["DetectionMode"].empty() || post_body["AlarmOnColour"].empty() || post_body["AlarmOffColour"].empty()) {
				return RetCode::API_INAVLID_ARGUMENT;
			}
			rule_type = motion;
			module_name = motion_module;
			coordinates = "";
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			canvas_size = canvas_size_str.c_str();
			label_str = post_body["Label"].get<std::string>();
			label = label_str.c_str();
			sensitivity = post_body["Sensitivity"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
		}
		else if (post_body["RuleType"].get<std::string>() =="CameraTamper" ) {
			rule_type = tamper;
			module_name = tamper_module;
			coordinates = "";
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			canvas_size = canvas_size_str.c_str();
			label = "";
			sensitivity = post_body["Sensitivity"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
		}
		else {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if (post_body["isActive"].get<std::string>() == "true") {
			active = 1;
		}


		char sql_query[INFO_BUFFER_LENGTH] = {0};

		memset(sql_query, '\0', 1);
		snprintf(sql_query, INFO_BUFFER_LENGTH, "UPDATE analytics_rules SET type = '%s', module_name = '%s', active = '%d', label = '%s', coordinates = '%s', sensitivity = '%f', invasion_time = '%f' ,behaviour = '%s', alarm_enable_colour = '%s', alarm_disable_colour = '%s', canvas_size = '%s', name = '%s' WHERE ID = '%d';", rule_type, module_name, active, label, coordinates, post_body["Sensitivity"].get<float>(), post_body["InvasionTime"].get<float>(), post_body["DetectionMode"].get<std::string>().c_str(), post_body["AlarmOnColour"].get<std::string>().c_str(), post_body["AlarmOffColour"].get<std::string>().c_str(), canvas_size, post_body["RuleName"].get<std::string>().c_str(), post_body["ID"].get<int>());
		sql_query[strlen(sql_query)] = '\0';
		// logger.log(Logger::INFO, sql_query);
		ret = hon_analytics_update_db(sql_query);
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		ret = hon_update_analytics_module(); //restart the analytics module for the changes to take effect
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		return RetCode::API_SUCCESS;
	}	
  	catch (const std::exception &e) {
		return RetCode::API_FAILURE;
  	}
}



RetCode AnalyticsApiHandler::createRule(const HttpRequest &Req, nlohmann::json &content) {
	try {

		int ret = 1;
		int active = 0;
		int rule_count = 0;
		const char *rule_type;
		const char *module_name;
		const char *coordinates;
		const char *canvas_size;
		const char *label;
		float sensitivity = 0.0;
		float invasion_time = 0.0;
		const char *behaviour;
		const char *alarm_enable_colour;
		const char *alarm_disable_colour;
		bool check_coord;
		bool zone_convextity;
		int coord_size ;
		char rule_names[BUFFER_LENGTH] = {0};
		std::string coordinates_str;
		std::string canvas_size_str;
		std::string label_str;
		std::string behaviour_str;
		std::string alarm_enable_colour_str;
		std::string alarm_disable_colour_str;
		// Logger& logger = Logger::getInstance();
		nlohmann::json post_body = Req.PostObject;
		if (post_body.empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if (post_body["RuleName"].empty() || post_body["isActive"].empty() || post_body["RuleType"].empty() || post_body["CanvasSize"].empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if(post_body["RuleName"].get<std::string>().length()<8) {
			return RetCode::RULE_NAME_LENGTH_FAILURE;
		}

		if(post_body["RuleName"].get<std::string>().length()>24) {
			return RetCode::RULE_NAME_LENGTH_FAILURE;
		}

		//regex to check if rulename contains special characters but space
		std::regex special_characters("[^a-zA-Z0-9_ -]");
		if (std::regex_search(post_body["RuleName"].get<std::string>(), special_characters)) {
			return RetCode::INVALID_RULE_NAME;
		}

		//regex to check if rulename starts with a number
		std::regex starts_with_number("^[0-9]");
		if (std::regex_search(post_body["RuleName"].get<std::string>(), starts_with_number)) {
			return RetCode::INVALID_RULE_NAME;
		}


		if (post_body["RuleType"].get<std::string>() =="Trespass" ) {
			if (post_body["Coordinates"].empty() || post_body["Label"].empty() || post_body["Sensitivity"].empty() || post_body["InvasionTime"].empty() || post_body["DetectionMode"].empty() || post_body["AlarmOnColour"].empty() || post_body["AlarmOffColour"].empty()) {
				return RetCode::API_INAVLID_ARGUMENT;
			}
			rule_type = trespass;
			module_name = trespass_module;
			// strcpy(coordinates,(char*)post_body["Coordinates"].get<std::string>().c_str());
			coordinates_str = post_body["Coordinates"].get<std::string>();
			check_coord = this->check_coordinates_format(coordinates_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			coord_size = get_coordinate_size(coordinates_str);
			if (coord_size < 3 || coord_size > 8) {
				return RetCode::TRESPASS_COORDINATE_SIZE_FAILURE;
			}
			coordinates = coordinates_str.c_str();
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			check_coord = this->check_coordinates_format(canvas_size_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			canvas_size = canvas_size_str.c_str();

			zone_convextity =  check_zone_convexity(coordinates_str);
			if (zone_convextity == true) {
				return RetCode::NON_CONVEX_COORDINATES;
			}

			label_str = post_body["Label"].get<std::string>();
			label = label_str.c_str();
			sensitivity = post_body["Sensitivity"].get<float>();
			invasion_time = post_body["InvasionTime"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
			// strcpy(label,(char*)post_body["Label"].get<std::string>().c_str());
			// logger.log(Logger::INFO, "from request",(char*)post_body["Label"].get<std::string>().c_str(), (char*)post_body["Coordinates"].get<std::string>().c_str());
			
		}
		else if (post_body["RuleType"].get<std::string>() == "Tripwire" ) {
			if (post_body["Coordinates"].empty() || post_body["Label"].empty() || post_body["Sensitivity"].empty() || post_body["DetectionMode"].empty() || post_body["AlarmOnColour"].empty() || post_body["AlarmOffColour"].empty()) {
				return RetCode::API_INAVLID_ARGUMENT;
			}
			rule_type = tripwire;
			module_name = tripwire_module;
			coordinates_str = post_body["Coordinates"].get<std::string>();
			check_coord = this->check_coordinates_format(coordinates_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			coord_size = get_coordinate_size(coordinates_str);
			if (coord_size != 2) {
				return RetCode::TRIPWIRE_COORDINATE_SIZE_FAILURE;
			}

			coordinates = coordinates_str.c_str();
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			check_coord = this->check_coordinates_format(canvas_size_str);
			if (check_coord == false) {
				return RetCode::API_COORDINATE_INVALID;
			}
			canvas_size = canvas_size_str.c_str();
			label_str = post_body["Label"].get<std::string>();
			label = label_str.c_str();
			sensitivity = post_body["Sensitivity"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
		}
		else if (post_body["RuleType"].get<std::string>() =="Motion" ) {
			if (post_body["Label"].empty() || post_body["Sensitivity"].empty() || post_body["DetectionMode"].empty() || post_body["AlarmOnColour"].empty() || post_body["AlarmOffColour"].empty()) {
				return RetCode::API_INAVLID_ARGUMENT;
			}
			rule_type = motion;
			module_name = motion_module;
			coordinates = "";
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			canvas_size = canvas_size_str.c_str();
			label_str = post_body["Label"].get<std::string>();
			label = label_str.c_str();
			sensitivity = post_body["Sensitivity"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
		}
		else if (post_body["RuleType"].get<std::string>() =="CameraTamper" ) {
			rule_type = tamper;
			module_name = tamper_module;
			coordinates = "";
			canvas_size_str = post_body["CanvasSize"].get<std::string>();
			canvas_size = canvas_size_str.c_str();
			label = "";
			sensitivity = post_body["Sensitivity"].get<float>();
			behaviour_str = post_body["DetectionMode"].get<std::string>();
			behaviour = behaviour_str.c_str();
			alarm_enable_colour_str = post_body["AlarmOnColour"].get<std::string>();
			alarm_enable_colour = alarm_enable_colour_str.c_str();
			alarm_disable_colour_str = post_body["AlarmOffColour"].get<std::string>();
			alarm_disable_colour = alarm_disable_colour_str.c_str();
		}
		else {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if (post_body["isActive"].get<std::string>() == "true") {
			active = 1;
		}


		char sql_query[INFO_BUFFER_LENGTH] = {0};
		char sql_count_query[INFO_BUFFER_LENGTH] = {0};
		char sql_query_rulename[INFO_BUFFER_LENGTH] = {0};
		// logger.log(Logger::INFO, post_body["RuleName"].get<std::string>().c_str(), rule_type, module_name, active, label, coordinates, sensitivity, invasion_time, behaviour, alarm_enable_colour, alarm_disable_colour);
		memset(sql_query, '\0', 1);
		snprintf(sql_count_query, INFO_BUFFER_LENGTH, "SELECT COUNT(*) FROM analytics_rules;");
		sql_count_query[strlen(sql_count_query)] = '\0';
		ret = hon_count_db_records(&rule_count, sql_count_query);
		if (ret!=0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		// logger.log(Logger::INFO, "Rule_count", rule_count);
		if (rule_count >= MAX_RULE_COUNT) {
			return RetCode::RULE_COUNT_EXCEEDS;
		}

		// rule_names = new char[LONG_INFO_BUFFER_LENGTH];
		// if (rule_names == NULL) {
		// 	return RetCode::API_MEM_ALLOC_FAILURE;
		// }
		memset(sql_query_rulename, '\0', 1);
		snprintf(sql_query_rulename, INFO_BUFFER_LENGTH, "SELECT name from analytics_rules;");
		sql_query_rulename[strlen(sql_query_rulename)] = '\0';
		// ret = hon_storage_get_recordings(&rule_names, sql_query_rulename);
		ret = hon_analytics_find_records(rule_names, sql_query_rulename);
		if (ret!=0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		nlohmann::json rule_name_json = nlohmann::json::parse(rule_names);
		// logger.log(Logger::INFO, "Rule___names", rule_name_json);
		
		for (int i = 0; i < rule_name_json["name"].size(); i++) {
			if (rule_name_json["name"].at(i).get<std::string>().compare(post_body["RuleName"].get<std::string>()) == 0) {
				return RetCode::RULE_NAME_EXISTS;
			}
		}
		

		memset(sql_query, '\0', 1);
		snprintf(sql_query, INFO_BUFFER_LENGTH, "INSERT INTO analytics_rules (name, type, module_name, active, label, coordinates, sensitivity, invasion_time,behaviour,alarm_enable_colour, alarm_disable_colour, canvas_size) VALUES ('%s', '%s', '%s', %d, '%s', '%s','%f','%f','%s','%s','%s','%s');", post_body["RuleName"].get<std::string>().c_str(), rule_type, module_name, active, label, coordinates, post_body["Sensitivity"].get<float>(), post_body["InvasionTime"].get<float>(),post_body["DetectionMode"].get<std::string>().c_str() , post_body["AlarmOnColour"].get<std::string>().c_str(), post_body["AlarmOffColour"].get<std::string>().c_str(), canvas_size);
		// logger.log(Logger::INFO, sql_query);
		sql_count_query[strlen(sql_count_query)] = '\0';
		ret = hon_analytics_update_db(sql_query);
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		ret = hon_update_analytics_module(); //restart the analytics module for the changes to take effect
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		return RetCode::API_SUCCESS;
	}	
  	catch (const std::exception &e) {
		return RetCode::API_FAILURE;
  	}
}


RetCode AnalyticsApiHandler::getRuleOptions(const HttpRequest &Req, nlohmann::json &content) {
	char *module_name;
	char *tmp;
	try {
		int ret = 1;
		RetCode ret_code;
		RetCode request_check = RetCode::API_REQUEST_CHECK_FAILURE;
		// Logger& logger = Logger::getInstance();
		// logger.log(Logger::INFO,"\n============comes in analytics_api.cpp getRuleOptions============\n");

		for (auto p : Req.Params) {
			if (p.Key == "moduleName") {
				module_name = (char*)malloc(SHORT_INFO_BUFFER_LENGTH);
				if (module_name == NULL) {
					return RetCode::API_MEM_ALLOC_FAILURE;
				}
				memset(module_name, '\0', 1);
				convert_space_in_param(p.Value);
				strncpy(module_name, p.Value.c_str(), SHORT_INFO_BUFFER_LENGTH);
				module_name[strlen(module_name)] = '\0';
				request_check = RetCode::API_REQUEST_CHECK_SUCCESS;
			}
		}

		if (module_name == NULL) {
				return RetCode::API_INAVLID_ARGUMENT;
		}

		tmp = new char[LONG_INFO_BUFFER_LENGTH];
		if (tmp == NULL) {
			return RetCode::API_MEM_ALLOC_FAILURE;
		}
		char sql_query[INFO_BUFFER_LENGTH] = {0};
		
		ret_code = getModuleOptions(Req, content);
		if (ret_code != RetCode::API_SUCCESS) {
			if (module_name) {
				free(module_name);
			}
			if (tmp) {
				delete[] tmp;
			}
			return RetCode::API_SQL_DB_FAILURE;
		}
		int check_module = 0;
		// logger.log(Logger::INFO,"\n=============content=======: ", content);
		// logger.log(Logger::INFO,"\n=============modulename=======: ", module_name);
		for (auto itr = content["ModuleName"].begin(); itr != content["ModuleName"].end(); ++itr) {
			if (strcmp(itr->get<std::string>().c_str(), module_name) == 0) {
				check_module = 1;
				break;
			}
		}

		if(check_module == 0) {
			content = nlohmann::json({});
			if (module_name) {
				free(module_name);
			}
			if (tmp) {
				delete[] tmp;
			}
			// logger.log(Logger::INFO,"\n=============check_module=======: ", check_module);
			return RetCode::API_INAVLID_ARGUMENT;
		}
		memset(sql_query, '\0', 1);
		snprintf(sql_query, INFO_BUFFER_LENGTH, "SELECT DISTINCT arb.behaviour_type AS DetectionMode FROM analytics_type AT JOIN analytics_rules_behaviour arb ON arb.rule_type = at.rule_type WHERE at.common_type = '%s';", module_name);
		sql_query[strlen(sql_query)] = '\0';
		
		ret = hon_analytics_find_records(tmp, sql_query);
		// logger.log(Logger::INFO,"\n============tmp============\n", tmp);
		if (ret != 0)
		{	
			if (module_name) {
				free(module_name);
			}
			if (tmp) {
				delete[] tmp;
			}
			return RetCode::API_SQL_DB_FAILURE;
		}
		nlohmann::json analytics_rule_options = nlohmann::json::parse(tmp);

		char labelmap_query[INFO_BUFFER_LENGTH] = {0};
		memset(labelmap_query, '\0', 1);
		snprintf(labelmap_query,INFO_BUFFER_LENGTH, "SELECT label from labelmap;");
		labelmap_query[strlen(labelmap_query)] = '\0';
		memset(tmp, '\0', 1);
		ret = hon_analytics_find_records(tmp, labelmap_query);
		if (ret != 0)
		{
			return RetCode::API_SQL_DB_FAILURE;
		}
		nlohmann::json labelmap = nlohmann::json::parse(tmp);


		// content = nlohmann::json({});
		content["DetectionMode"] = analytics_rule_options["DetectionMode"];
		content["AlarmOnColour"] = "red";//analytics_rule_options["alarm_enable_colour"].at(0);
		content["AlarmOffColour"] = "red";//analytics_rule_options["alarm_disable_colour"].at(0);
		content["Label"] = labelmap["label"]; //{"person","bag","two-wheeler","four-wheeler","animal","number_plate"};
		if (module_name) {
			free(module_name);
		} 
		if (tmp) {
			delete[] tmp;
		}	
		return RetCode::API_SUCCESS;
  	} 
  catch (const std::exception &e) {
	if (module_name) {
		free(module_name);
	}
	if (tmp) {
		delete[] tmp;
	}
	// delete[] tmp;
	content = nlohmann::json({});
	return RetCode::API_FAILURE;
  }
}


RetCode AnalyticsApiHandler::getModuleOptions(const HttpRequest &Req, nlohmann::json &content) {
	char *tmp;
	try {
		int ret = 1;
		tmp = new char[LONG_INFO_BUFFER_LENGTH];
		if (tmp == NULL) {
			return RetCode::API_MEM_ALLOC_FAILURE;
		}
		char sql_query[INFO_BUFFER_LENGTH] = {0};
		snprintf(sql_query, INFO_BUFFER_LENGTH, "SELECT DISTINCT common_type as ModuleName from analytics_type;");
		sql_query[strlen(sql_query)] = '\0';
		ret = hon_analytics_find_records(tmp, sql_query);
		if (ret!=0)
		{
			return RetCode::API_SQL_DB_FAILURE;
		}
		nlohmann::json analytics_rule_options = nlohmann::json::parse(tmp);
		// content["= nlohmann::json({});
		content["ModuleName"] = analytics_rule_options["ModuleName"];
		if (tmp) {
			delete[] tmp;
		}
		return RetCode::API_SUCCESS;
	}
	catch (const std::exception &e) {
		if (tmp) {
			delete[] tmp;
		}
		return RetCode::API_FAILURE;
	}
}

RetCode AnalyticsApiHandler::deleteRules(const HttpRequest &Req, nlohmann::json &content) {

	try 
	{
		int ret = 1;

		ret = hon_stop_analytics_module(); //restart the analytics module for the changes to take effect
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		
		nlohmann::json post_body = Req.PostObject;
		if (post_body.empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		if (post_body["RuleName"].empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}

		
		std::string rule_list = "";
		int rule_count = post_body["RuleName"].size();
		rule_list = rule_list+"(";
		int count = 1;
		for (auto itr = post_body["RuleName"].begin(); itr!=post_body["RuleName"].end();++itr,++count)
		{	
			rule_list = rule_list+"'"+itr->get<std::string>()+"'";
			if (count != rule_count) {
				rule_list = rule_list+",";
			}
		}

		rule_list = rule_list+")";
		// rule_list : "('rule1','rule2','rule3')"

		char sql_query[INFO_BUFFER_LENGTH] = {0};
		memset(sql_query, '\0', 1);
		snprintf(sql_query,INFO_BUFFER_LENGTH, "DELETE FROM analytics_rules WHERE name IN %s;", rule_list.c_str());
		sql_query[strlen(sql_query)] = '\0';
		ret = hon_analytics_update_db(sql_query);

		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		
		ret = hon_start_analytics_module(); //restart the analytics module for the changes to take effect
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		return RetCode::API_SUCCESS;

	}
	catch (const std::exception &e) {
		return RetCode::API_FAILURE;
	}


}


bool AnalyticsApiHandler::check_coordinates_format(const std::string& input) {
    std::regex coord_pattern(R"(\[(\[-?\d+(\.\d+)?,\s*-?\d+(\.\d+)?\])(,\s*\[-?\d+(\.\d+)?,\s*-?\d+(\.\d+)?\])*\])");    
    return std::regex_match(input, coord_pattern);
}

std::vector<std::pair<int,int>> AnalyticsApiHandler::parse_coordinates(const std::string& str) {
	
    std::vector<std::pair<int,int>> coordinates;
    std::regex re(R"(\[\s*([0-9]+)\s*,\s*([0-9]+)\s*\])");
    std::smatch match;
	std::string s = str;
    while (std::regex_search(s, match, re)) {
        int x = std::stoi(match[1].str());
        int y = std::stoi(match[2].str());
        //TODO: FIXME: Hardcoded values for BASE frame width and height -> Done
        coordinates.push_back({x, y});
        s = match.suffix();
    }
    return coordinates;
}


RetCode AnalyticsApiHandler::getCameraTamperingOptions(const HttpRequest &Req, nlohmann::json &content) {
	char *tmp;
	try {
		int ret = 1;
		int camera_tamper_active = 0;
		RetCode ret_code;
		// Logger& logger = Logger::getInstance();
		// logger.log(Logger::INFO,"\n============comes in analytics_api.cpp get cam tampering options============\n");

		tmp = new char[LONG_INFO_BUFFER_LENGTH];
		char sql_query[INFO_BUFFER_LENGTH] = {0};
		
		memset(sql_query, '\0', 1);
		snprintf(sql_query, INFO_BUFFER_LENGTH, "SELECT * from camera_tampering_module;");
		sql_query[strlen(sql_query)] = '\0';
		
		ret = hon_analytics_find_records(tmp, sql_query);
		// logger.log(Logger::INFO,"\n============tmp============\n", tmp);
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		nlohmann::json camera_tamper_options = nlohmann::json::parse(tmp);
		// content["Result"] = nlohmann::json({});
		for (int i =0 ; i<camera_tamper_options["behaviour"].size();i++)
		{
			if (camera_tamper_options["behaviour"].at(i).get<std::string>().compare("image_too_bright") == 0)
			{
				content["ImageTooBrightIsActive"] = std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
				content["ImageTooBrightSensitivity"] = std::stoi(camera_tamper_options["sensitivity"].at(i).get<std::string>());
				camera_tamper_active += std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
			}
			else if(camera_tamper_options["behaviour"].at(i).get<std::string>().compare("image_too_dark") == 0)
			{
				content["ImageTooDarkIsActive"] = std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
				content["ImageTooDarkSensitivity"] = std::stoi(camera_tamper_options["sensitivity"].at(i).get<std::string>());
				camera_tamper_active += std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
			}
			else if(camera_tamper_options["behaviour"].at(i).get<std::string>().compare("image_too_blurry") == 0)
			{
				content["ImageTooBlurryIsActive"] = std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
				content["ImageTooBlurrySensitivity"] = std::stoi(camera_tamper_options["sensitivity"].at(i).get<std::string>());
				camera_tamper_active += std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
			}
			else if(camera_tamper_options["behaviour"].at(i).get<std::string>().compare("global_scene_change") == 0)
			{
				content["GlobalSceneChangeIsActive"] = std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
				content["GlobalSceneChangeSensitivity"] = std::stoi(camera_tamper_options["sensitivity"].at(i).get<std::string>());
				camera_tamper_active += std::stoi(camera_tamper_options["active"].at(i).get<std::string>());
			}
		}
		if (camera_tamper_active > 0) {
			content["VideoTamperingIsActive"] = 1;
		}
		else {
			content["VideoTamperingIsActive"] = 0;
		}
		// content["Result"]["Behaviour"] = camera_tamper_options["behaviour"];
		delete[] tmp;
		return RetCode::API_SUCCESS;
  	} 
  catch (const std::exception &e) {
	if (tmp) {
		delete[] tmp;
	}
	// delete[] tmp;
	return RetCode::API_FAILURE;
  }
}



RetCode AnalyticsApiHandler::setCameraTamperingOptions(const HttpRequest &Req, nlohmann::json &content) {
	char *tmp;
	int *active;
	char *behaviour;
	try {
		int ret = 1;
		int key_count = 0;
		RetCode ret_code;
		// Logger& logger = Logger::getInstance();

		nlohmann::json post_body = Req.PostObject;
		if (post_body.empty()) {
			return RetCode::API_INAVLID_ARGUMENT;
		}


		if (post_body["VideoTamperingIsActive"].empty() || post_body["ImageTooBrightIsActive"].empty() || post_body["ImageTooDarkIsActive"].empty() || post_body["ImageTooBlurryIsActive"].empty() || post_body["GlobalSceneChangeIsActive"].empty() || post_body["ImageTooBrightSensitivity"].empty() || post_body["ImageTooDarkSensitivity"].empty() || post_body["ImageTooBlurrySensitivity"].empty() || post_body["GlobalSceneChangeSensitivity"].empty()) {
				return RetCode::API_INAVLID_ARGUMENT;
		}


		char sql_query[INFO_BUFFER_LENGTH] = {0};
		memset(sql_query, '\0', 1);

		char base_query[] = "UPDATE camera_tampering_module "
							"SET "
							"active = CASE "
								"WHEN behaviour = 'image_too_bright' THEN %d "
								"WHEN behaviour = 'image_too_dark' THEN %d "
								"WHEN behaviour = 'image_too_blurry' THEN %d "
								"WHEN behaviour = 'global_scene_change' THEN %d "
								"ELSE active "
							"END, "
							"sensitivity = CASE "
								"WHEN behaviour = 'image_too_bright' THEN %d "
								"WHEN behaviour = 'image_too_dark' THEN %d "
								"WHEN behaviour = 'image_too_blurry' THEN %d "
								"WHEN behaviour = 'global_scene_change' THEN %d "
								"ELSE sensitivity "
							"END;";




		// snprintf(sql_query, INFO_BUFFER_LENGTH, base_query, post_body["Behaviour"][this->camera_tamper_type["image_too_bright"]]["isActive"].get<int>(),
		// 												   post_body["Behaviour"][this->camera_tamper_type["image_too_dark"]]["isActive"].get<int>(),
		// 												   post_body["Behaviour"][this->camera_tamper_type["image_too_blurry"]]["isActive"].get<int>(),
		// 												   post_body["Behaviour"][this->camera_tamper_type["global_scene_change"]]["isActive"].get<int>(),
		// 												   post_body["Behaviour"][this->camera_tamper_type["image_too_bright"]]["Sensitivity"].get<int>(),
		// 												   post_body["Behaviour"][this->camera_tamper_type["image_too_dark"]]["Sensitivity"].get<int>(),
		// 												   post_body["Behaviour"][this->camera_tamper_type["image_too_blurry"]]["Sensitivity"].get<int>(),
		// 												   post_body["Behaviour"][this->camera_tamper_type["global_scene_change"]]["Sensitivity"].get<int>()
		// 						);


		snprintf(sql_query, INFO_BUFFER_LENGTH, base_query, post_body["ImageTooBrightIsActive"].get<int>(),
														   post_body["ImageTooDarkIsActive"].get<int>(),
														   post_body["ImageTooBlurryIsActive"].get<int>(),
														   post_body["GlobalSceneChangeIsActive"].get<int>(),
														   post_body["ImageTooBrightSensitivity"].get<int>(),
														   post_body["ImageTooDarkSensitivity"].get<int>(),
														   post_body["ImageTooBlurrySensitivity"].get<int>(),
														   post_body["GlobalSceneChangeSensitivity"].get<int>()
								);


		
		

		sql_query[strlen(sql_query)] = '\0';
		
		ret = hon_analytics_update_db(sql_query);
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}
		ret = hon_update_camera_tamper_module(); //restart the camera_tamper module for the changes to take effect
		if (ret != 0) {
			return RetCode::API_SQL_DB_FAILURE;
		}

		return RetCode::API_SUCCESS;
  	} 
  catch (const std::exception &e) {
	// delete[] tmp;
	return RetCode::API_FAILURE;
  }
}



bool AnalyticsApiHandler::check_line_intersection(std::pair<int, int>& p11, std::pair<int, int>& p12, std::pair<int, int>& p21, std::pair<int, int>& p22) 
{
    int tc1 = (p11.first - p12.first) * (p21.second - p11.second) + (p11.second - p12.second) * (p11.first - p21.first);
    int tc2 = (p11.first - p12.first) * (p22.second - p11.second) + (p11.second - p12.second) * (p11.first - p22.first);
    int td1 = (p21.first - p22.first) * (p11.second - p21.second) + (p21.second - p22.second) * (p21.first - p11.first);
    int td2 = (p21.first - p22.first) * (p12.second - p21.second) + (p21.second - p22.second) * (p21.first - p12.first);
    return tc1 * tc2 < 0 && td1 * td2 < 0;
}



int AnalyticsApiHandler::get_coordinate_size(std::string input_str) {
	std::vector<std::pair<int,int>> coordinates;
    std::regex re(R"(\[\s*([0-9]+)\s*,\s*([0-9]+)\s*\])");
    std::smatch match;

    while (std::regex_search(input_str, match, re)) {
        int x = std::stoi(match[1].str());
        int y = std::stoi(match[2].str());
        coordinates.push_back({x, y});
        input_str = match.suffix();
    }
	int size = coordinates.size();
	return size;
}


// function to check if point q lies on line segment 'pr'
bool onSegment(std::vector<int>& p, std::vector<int>& q, std::vector<int>& r) {
    return (q[0] <= std::max(p[0], r[0]) && 
            q[0] >= std::min(p[0], r[0]) &&
            q[1] <= std::max(p[1], r[1]) && 
            q[1] >= std::min(p[1], r[1]));
}

// function to find orientation of ordered triplet (p, q, r)
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(std::vector<int>& p, std::vector<int>& q, std::vector<int>& r) {
    int val = (q[1] - p[1]) * (r[0] - q[0]) -
              (q[0] - p[0]) * (r[1] - q[1]);

    // collinear
    if (val == 0) return 0;

    // clock or counterclock wise
    // 1 for clockwise, 2 for counterclockwise
    return (val > 0) ? 1 : 2;
}


// function to check if two line segments intersect
bool doIntersect(std::vector<std::vector<std::vector<int>>>& points) {

    // find the four orientations needed
    // for general and special cases
    int o1 = orientation(points[0][0], points[0][1], points[1][0]);
    int o2 = orientation(points[0][0], points[0][1], points[1][1]);
    int o3 = orientation(points[1][0], points[1][1], points[0][0]);
    int o4 = orientation(points[1][0], points[1][1], points[0][1]);

    // general case
    if (o1 != o2 && o3 != o4)
        return true;

    // special cases
    // p1, q1 and p2 are collinear and p2 lies on segment p1q1
    if (o1 == 0 && 
    onSegment(points[0][0], points[1][0], points[0][1])) return true;

    // p1, q1 and q2 are collinear and q2 lies on segment p1q1
    if (o2 == 0 && 
    onSegment(points[0][0], points[1][1], points[0][1])) return true;

    // p2, q2 and p1 are collinear and p1 lies on segment p2q2
    if (o3 == 0 && 
    onSegment(points[1][0], points[0][0], points[1][1])) return true;

    // p2, q2 and q1 are collinear and q1 lies on segment p2q2 
    if (o4 == 0 && 
    onSegment(points[1][0], points[0][1], points[1][1])) return true;

    return false;
}


	bool check_collinearity(std::pair<int,int>& p1, std::pair<int,int>& p2, std::pair<int,int>& p3) {
	
		int x1 = p1.first;
		int y1 = p1.second;
		int x2 = p2.first;
		int y2 = p2.second;
		int x3 = p3.first;
		int y3 = p3.second;

		if ((y2 - y1) * (x3 - x2) == (y3 - y2) * (x2 - x1)) {
			return true;
		}
		return false;
	}

	bool AnalyticsApiHandler::check_zone_convexity(std::string s) {

		std::vector<std::pair<int,int>> coordinates;
		std::regex re(R"(\[\s*([0-9]+)\s*,\s*([0-9]+)\s*\])");
		std::smatch match;
		bool collinearity = false;
		while (std::regex_search(s, match, re)) {
			int x = std::stoi(match[1].str());
			int y = std::stoi(match[2].str());
			coordinates.push_back({x, y});
			s = match.suffix();
		}
		if (coordinates.size() == 3) {
			collinearity =  check_collinearity(coordinates.at(0), coordinates.at(1), coordinates.at(2));
			if (collinearity == true) {
				return true;
			}
		}
		std::vector<std::vector<int>> line_segments;
		int count = 1;
		int x1,y1,x2,y2;
		for(auto itr1 = coordinates.begin(); itr1!= coordinates.end(); itr1++) {
			x1 = itr1->first;
			y1 = itr1->second;
			x2 = coordinates.at(count).first;
			y2 = coordinates.at(count).second; 
			std::vector<int> line_segment = {x1,y1,x2,y2};
			line_segments.push_back(line_segment);
			count++;
			count = count % coordinates.size();
		}
		// print_line_segments(line_segments);
			
		int intersetc_count = 0;
		for (int i=0;i<line_segments.size();i++)
		{   
			intersetc_count = 0;
			std::cout<<line_segments.at(i).at(0)<<","<<line_segments.at(i).at(1)<<","<<line_segments.at(i).at(2)<<","<<line_segments.at(i).at(3)<<std::endl;
			for (int j=i+1;j<line_segments.size();j++)
			{
				if (i==0 && j==line_segments.size()-1) {
					continue;
				}
				std::vector<int> p11 = {line_segments.at(i).at(0), line_segments.at(i).at(1)};
				std::vector<int> p12 = {line_segments.at(i).at(2), line_segments.at(i).at(3)};
				std::vector<int> p21 = {line_segments.at(j).at(0), line_segments.at(j).at(1)};
				std::vector<int> p22 = {line_segments.at(j).at(2), line_segments.at(j).at(3)};
				std::vector<std::vector<std::vector<int>>> points = {{p11, p12}, {p21, p22}};
				if (doIntersect(points)) {
					// print line segments which intersect
					intersetc_count++;
					// std::cout<<line_segments.at(j).at(0)<<","<<line_segments.at(j).at(1)<<","<<line_segments.at(j).at(2)<<","<<line_segments.at(j).at(3)<<std::endl;
					if (intersetc_count >=2) {
						return true;
					}
				}
			}
		}
		return false;
	}




} // namespace cgi
} // namespace rockchip
