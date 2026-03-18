// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "event_api.h"
#include "common.h"
#include <time.h>
#include <unistd.h>

namespace rockchip {
namespace cgi {

int GetScheduleId(std::string schedule_name) {
  if (!schedule_name.compare("motion")) {
    return 0;
  } else if (!schedule_name.compare("intrusion")) {
    return 1;
  } else if (!schedule_name.compare("video-plan")) {
    return 2;
  } else if (!schedule_name.compare("screenshot")) {
    return 3;
  } else {
    return -1;
  }
}


nlohmann::json get_schedule_slots() {
  char *tmp = new char[5000]; // Allocate enough memory for the JSON string
  int ret = 0;
  nlohmann::json schedule_slots = nlohmann::json::object(); // Use an object to group by day_of_week
  nlohmann::json tmp_json;
  char sql_query[] = "SELECT * FROM schedule_slots;";

  // Call the function to get the JSON structure
  ret = hon_storage_get_schedule_plan(tmp, sql_query);
  if (ret != 0) {
      if (tmp) {
          delete[] tmp;
      }
      return schedule_slots; // Return an empty JSON object on failure
  }

  try {
      // Parse the JSON string into a nlohmann::json object
      tmp_json = nlohmann::json::parse(tmp);
  } catch (const std::exception &e) {
      // LOG_ERROR("JSON parsing failed: %s\n", e.what());
      if (tmp) {
          delete[] tmp;
      }
      return schedule_slots; // Return an empty JSON object on parsing failure
  }

  // Iterate through the parsed JSON and group slots by day_of_week
  for (int i = 0; i < tmp_json["day_of_week"].size(); i++) {
      std::string day = tmp_json["day_of_week"].at(i).get<std::string>();
      nlohmann::json slot;
      slot["slot_id"] = std::stoi(tmp_json["slot_number"].at(i).get<std::string>());
      slot["st"] = std::stoi(tmp_json["start_time"].at(i).get<std::string>());
      slot["et"] = std::stoi(tmp_json["end_time"].at(i).get<std::string>());
      slot["enabled"] = std::stoi(tmp_json["enabled"].at(i).get<std::string>());

      // Add the slot to the corresponding day
      if (schedule_slots.find(day) != schedule_slots.end()) {
          schedule_slots[day].push_back(slot);
      } else {
          schedule_slots[day] = nlohmann::json::array();
          schedule_slots[day].push_back(slot);
      }
  }

  // Free the allocated memory
  if (tmp) {
      delete[] tmp;
  }

  return schedule_slots;
}
using json = nlohmann::json;
void set_schedule_slots(const json &payload) {
  // Start constructing the SQL query
  std::string sql_query = "BEGIN TRANSACTION; ";

  // Iterate over the JSON payload to construct the SQL query
  for (auto it = payload.begin(); it != payload.end(); ++it) {
      const std::string &day_of_week = it.key();
      const json &slots = it.value();
      if (!slots.is_array()) {
          // Skip invalid slots format
          continue;
      }

      // Iterate over the slots for the current day
      for (const auto &slot : slots) {
          int slot_number = slot.at("slot_id").get<int>();
          int start_time = slot.at("st").get<int>();
          int end_time = slot.at("et").get<int>();
          int enabled = slot.at("enabled").get<int>();

          // Append the SQL statement for this slot
          sql_query += "INSERT INTO schedule_slots (day_of_week, slot_number, start_time, end_time, enabled) "
                       "VALUES ('" + day_of_week + "', " +
                       std::to_string(slot_number) + ", " +
                       std::to_string(start_time) + ", " +
                       std::to_string(end_time) + ", " +
                       std::to_string(enabled) + ") "
                       "ON CONFLICT(day_of_week, slot_number) "
                       "DO UPDATE SET start_time = " + std::to_string(start_time) +
                       ", end_time = " + std::to_string(end_time) +
                       ", enabled = " + std::to_string(enabled) + "; ";
      }
  }

  // End the transaction
  sql_query += "COMMIT;";

  // Execute the SQL query using hon_storage_set_schedule_plan
  hon_storage_set_schedule_plan(sql_query.c_str());
}

nlohmann::json get_recordings_for_day(const std::string &date) {
  char *tmp = nullptr; // Pointer to hold the dynamically allocated JSON string
  int ret = 0;

  try {
      // Prepare the SQL query with the input date
      std::string sql_query = "SELECT filename, start_time, end_time, recording_type, status FROM recordings WHERE date = '" + date + "';";

      // Call the function to get the JSON structure
      ret = hon_storage_get_recordings(&tmp, sql_query.c_str()); // Pass the address of tmp
      if (ret != 0 || tmp == nullptr) {
          throw std::runtime_error("No records found or error occurred in hon_storage_get_recordings");
      }

      // Directly parse the JSON string into a nlohmann::json object
      nlohmann::json recordings_for_day = nlohmann::json::parse(tmp);

      // Free the allocated memory
      if (tmp) {
          free(tmp);
      }

      return recordings_for_day;
  } catch (const std::exception &e) {
      // LOG_ERROR("Error in get_recordings_for_day: %s\n", e.what());
      if (tmp) {
          free(tmp); // Free the dynamically allocated memory
      }
      return nlohmann::json::array(); // Return an empty JSON array on failure
  }
}

nlohmann::json get_recordings_for_time_slot(const std::string &date, int starttime, int endtime) {
  char *tmp = nullptr; // Pointer to hold the dynamically allocated JSON string
  int ret = 0;

  try {
      // Prepare the SQL query with the input date, starttime, and endtime
      std::string sql_query = "SELECT filename, start_time, end_time, recording_type, status "
                              "FROM recordings "
                              "WHERE date = '" + date + "' AND start_time >= " + std::to_string(starttime) +
                              " AND end_time <= " + std::to_string(endtime) + ";";

      // Call the function to get the JSON structure
      ret = hon_storage_get_recordings(&tmp, sql_query.c_str()); // Pass the address of tmp
      if (ret != 0 || tmp == nullptr) {
          throw std::runtime_error("No recordings found or error occurred in hon_storage_get_recordings");
      }

      // Directly parse the JSON string into a nlohmann::json object
      nlohmann::json recordings_for_time_slot = nlohmann::json::parse(tmp);

      // Free the allocated memory
      if (tmp) {
          free(tmp);
      }

      return recordings_for_time_slot;
  } catch (const std::exception &e) {
      // LOG_ERROR("Error in get_recordings_for_time_slot: %s\n", e.what());
      if (tmp) {
          free(tmp); // Free the dynamically allocated memory
      }
      return nlohmann::json::array(); // Return an empty JSON array on failure
  }
}


nlohmann::json get_recordings_for_month(const std::string &month) {
  char *tmp = nullptr; // Pointer to hold the dynamically allocated JSON string
  int ret = 0;

  try {
      // Prepare the SQL query to get distinct days for the specified month
      std::string sql_query = "SELECT DISTINCT date FROM recordings WHERE date LIKE '" + month + "-%';";

      // Call the function to get the JSON structure
      ret = hon_storage_get_recordings(&tmp, sql_query.c_str()); // Pass the address of tmp
      if (ret != 0 || tmp == nullptr) {
          throw std::runtime_error("No recordings found or error occurred in hon_storage_get_recordings");
      }

      // Parse the JSON string into a nlohmann::json object
      nlohmann::json tmp_json = nlohmann::json::parse(tmp);

      // Transform the JSON structure to aggregate all dates under a single "date" key
      nlohmann::json recordings_for_month;
      recordings_for_month["dates"] = nlohmann::json::array(); // Initialize the "date" array

      for (const auto &record : tmp_json) {
          recordings_for_month["dates"].push_back(record.at("date").get<std::string>());
      }

      // Free the allocated memory
      if (tmp) {
          free(tmp);
      }

      return recordings_for_month;
  } catch (const std::exception &e) {
      // LOG_ERROR("Error in get_recordings_for_month: %s\n", e.what());
      if (tmp) {
          free(tmp); // Free the dynamically allocated memory
      }
      return nlohmann::json::object(); // Return an empty JSON object on failure
  }
}

nlohmann::json regional_invasion_get() {
  int value;
  char *str;
  nlohmann::json ret;
  nlohmann::json regional_invasion;
  nlohmann::json normalized_screen_size;

  hon_osd_get_normalized_screen_width(&value);
  normalized_screen_size.emplace("iNormalizedScreenWidth", value);
  hon_osd_get_normalized_screen_height(&value);
  normalized_screen_size.emplace("iNormalizedScreenHeight", value);

  hon_event_ri_get_enabled(&value);
  regional_invasion.emplace("iEnabled", value);
  hon_event_ri_get_position_x(&value);
  regional_invasion.emplace("iPositionX", value);
  hon_event_ri_get_position_y(&value);
  regional_invasion.emplace("iPositionY", value);
  hon_event_ri_get_width(&value);
  regional_invasion.emplace("iWidth", value);
  hon_event_ri_get_height(&value);
  regional_invasion.emplace("iHeight", value);
  hon_event_ri_get_proportion(&value);
  regional_invasion.emplace("iProportion", value);
  hon_event_ri_get_sensitivity_level(&value);
  regional_invasion.emplace("iSensitivityLevel", value);
  hon_event_ri_get_time_threshold(&value);
  regional_invasion.emplace("iTimeThreshold", value);

  ret.emplace("normalizedScreenSize", normalized_screen_size);
  ret.emplace("regionalInvasion", regional_invasion);

  return ret;
}

void EventApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  std::string path_api_resource;
  std::string path_specific_resource;
  std::string path_channel_resource;
  char *str;
  nlohmann::json content;

  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_specific_resource.find_first_of("/");
    if (pos_first != -1) {
      path_channel_resource = path_specific_resource.substr(
          pos_first + 1, path_specific_resource.size());
    }
  }
  if (Req.Method == "GET") {
    #ifdef TEST
    if (path_specific_resource.find("triggers") != std::string::npos) {
      // get triggers info
      if (!path_channel_resource.compare("vmd_0")) {
        content = R"(
                      {
                        "iNotificationCenterEnabled": 0,
                        "iNotificationEmailEnabled": 0,
                        "iNotificationFTPEnabled": 0,
                        "iNotificationIO1Enabled": 0,
                        "iNotificationRecord1Enabled": 0,
                        "iVideoInputChannelID": 0,
                        "id": 0,
                        "sEventType": "VMD"
                      }
                    )"_json;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (!path_channel_resource.compare("vri_0")) {
        content = R"(
                      {
                        "iNotificationCenterEnabled": 0,
                        "iNotificationEmailEnabled": 0,
                        "iNotificationFTPEnabled": 0,
                        "iNotificationIO1Enabled": 0,
                        "iNotificationRecord1Enabled": 0,
                        "iVideoInputChannelID": 0,
                        "id": 1,
                        "sEventType": "VRI"
                      }
                    )"_json;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    } else if (path_specific_resource.find("schedules") != std::string::npos) {
      // get schedules info
      if (!path_channel_resource.empty()) {
        int para_id = GetScheduleId(path_channel_resource);
        // minilog_debug("id is %d", para_id);
        if (para_id >= 0) {
          content = R"(
                        {
                          "sSchedulesJson": "[[],[],[],[],[],[],[]]"
                        }
                      )"_json;
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(content);
        } else {
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        }
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    } else if (path_specific_resource.find("motion-detection") !=
               std::string::npos) {
      // get motion detection info
      if (!path_channel_resource.compare("0")) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    } else if (path_specific_resource.find("regional-invasion") !=
               std::string::npos) {
      // get regional invasion info
      if (!path_channel_resource.compare("0")) {
        content = regional_invasion_get();
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    } else if (!path_specific_resource.compare("face-list")) {
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else if (path_specific_resource.find("face/") != std::string::npos) {
      // get face info
      if (!path_channel_resource.empty()) {
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      }
    } else if (!path_specific_resource.compare("face-config")) {
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else if (!path_specific_resource.compare("face-waiting")) {
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else if (path_specific_resource.find("smart") != std::string::npos) {
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else if (!path_specific_resource.compare("last-face")) {
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else if (!path_specific_resource.compare("get-record-status")) {
      int record_status;
      hon_storage_record_statue_get(&record_status);
      content = record_status;
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else {
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
    #endif
    if (!path_specific_resource.compare("schedules")) {
      // get schedules info
        content = get_schedule_slots();
				if (!content.empty()) 
				{
					Resp.setHeader(HttpStatus::kOk, "OK");
					Resp.setApiData(HttpStatus::kOk,content,"Schedules retrieved successfully");
					return;
				} else
				{
					Resp.setHeader(HttpStatus::kBadRequest, "No Slots enabled");
					Resp.setApiData(HttpStatus::kBadRequest,content,"No Slots enabled");
					return;
        }
    }
    
    if (!path_specific_resource.compare("recordings")) {
      // Extract the query string from the request
      if (Req.Params.empty()) {
          Resp.setApiData(HttpStatus::kBadRequest, content, "Bad Request Params");
          Resp.setHeader(HttpStatus::kBadRequest, "Bad Request Params");
          return;
      }
  
      std::string date;
      std::string month;
      int starttime = -1; // Default value indicating no starttime provided
      int endtime = -1;   // Default value indicating no endtime provided
      

      //date is the mandatory parameter
      //month is the optional parameter
      //starttime and endtime are optional parameters
      //starttime and endtime are used to filter the recordings for a specific time slot
      for (auto p : Req.Params) {
          if (p.Key == "date") {
              date = p.Value;
          } else if( p.Key == "month") {
              month = p.Value;
          } else if (p.Key == "st") {
              starttime = std::stoi(p.Value); // Convert starttime to integer
          } else if (p.Key == "et") {
              endtime = std::stoi(p.Value);   // Convert endtime to integer
          }
      }
      // 1. Handle month parameter (highest priority)
      if (!month.empty()) {
        // Handle recordings for a specific month
        if (month.length() != 7 || month[4] != '-') {
            Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid month format");
            Resp.setHeader(HttpStatus::kBadRequest, "Invalid month format");
            return;
        }

        // Call the function to get recordings for the specified month
        content = get_recordings_for_month(month);
        if (!content.empty()) {
            Resp.setHeader(HttpStatus::kOk, "OK");
            Resp.setApiData(HttpStatus::kOk, content, "Recordings dates retrieved successfully");
        } else {
            Resp.setApiData(HttpStatus::kBadRequest, content, "No recordings found for the specified month");
            Resp.setHeader(HttpStatus::kBadRequest, "No recordings found for the specified month");
        }
        return;
      }

      // 2. Handle date parameter (second priority)
      if (date.empty()) {
          Resp.setApiData(HttpStatus::kBadRequest, content, "Date parameter is required");
          Resp.setHeader(HttpStatus::kBadRequest, "Date parameter is required");
          return;
      }

      //Check date format
      if (date.length() != 10 || date[4] != '-' || date[7] != '-') {
          Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid date format");
          Resp.setHeader(HttpStatus::kBadRequest, "Invalid date format");
          return;
      }
      
      // 2. Handle `date` with `st` and `et` (time slot query)
      if (starttime != -1 && endtime != -1) {
          // Handle recordings for a specific time slot
          if (starttime >= endtime) {
              Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid time slot");
              Resp.setHeader(HttpStatus::kBadRequest, "Invalid time slot");
              return;
          }
          // Ensure starttime and endtime are within valid range
          //HHMMSS
          if (starttime <= 0 || starttime >= 235959 || endtime <= 0 || endtime >= 235959) {
              Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid time format");
              Resp.setHeader(HttpStatus::kBadRequest, "Invalid time format");
              return;
          }
          // Call the function to get recordings for the specified time slot
          // and date
          content = get_recordings_for_time_slot(date, starttime, endtime);
          if (!content.empty()) {
              Resp.setHeader(HttpStatus::kOk, "OK");
              Resp.setApiData(HttpStatus::kOk, content, "Recordings retrieved successfully");
          } else {
              Resp.setApiData(HttpStatus::kBadRequest, content, "No recordings found for the specified time slot");
              Resp.setHeader(HttpStatus::kBadRequest, "No recordings found for the specified time slot");
              return;
          }
      } else {
          // Handle recordings for the entire day
          content = get_recordings_for_day(date);
          if (!content.empty()) {
              Resp.setHeader(HttpStatus::kOk, "OK");
              Resp.setApiData(HttpStatus::kOk, content, "Recordings retrieved successfully");
          } else {
              Resp.setHeader(HttpStatus::kBadRequest, "No recordings found for the specified day");
              Resp.setApiData(HttpStatus::kOk, content, "No recordings found for the specified day");
              return;
          }
      }
    }
     else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    }
   
  else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
 
    nlohmann::json event_config = Req.PostObject; /* must be json::object */
    char *config = new char[Req.ContentLength + 1];
    strncpy(config, event_config.dump().c_str(), Req.ContentLength);
    config[Req.ContentLength] = '\0';

    if (path_specific_resource.find("schedules") != std::string::npos) {
      // set schedules info
        set_schedule_slots(event_config);
        content = get_schedule_slots();
        if (!content.empty()) {
            Resp.setHeader(HttpStatus::kOk, "OK");
            Resp.setApiData(HttpStatus::kOk, content, "Schedules updated successfully");
            return;
        } else {
            Resp.setHeader(HttpStatus::kBadRequest, "No Slots enabled");
            Resp.setApiData(HttpStatus::kBadRequest, content, "No Slots enabled");
            return;
        }
    } 
    else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
    #ifdef TEST
    if (path_specific_resource.find("triggers/") != std::string::npos) {

      if (!path_channel_resource.compare("vmd_0")) {
        // // set
        // dbserver_event_set((char *)TABLE_EVENT_TRIGGERS, config, 0);
        // mediaserver_sync_schedules();
        // // get new info
        // str = dbserver_event_get((char *)TABLE_EVENT_TRIGGERS);
        // content = nlohmann::json::parse(str).at("jData").at(0);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (!path_channel_resource.compare("vri_0")) {
        // // set
        // dbserver_event_set((char *)TABLE_EVENT_TRIGGERS, config, 1);
        // // get new info
        // str = dbserver_event_get((char *)TABLE_EVENT_TRIGGERS);
        // content = nlohmann::json::parse(str).at("jData").at(1);
        content = R"(
                      {
                        "iNotificationCenterEnabled": 0,
                        "iNotificationEmailEnabled": 0,
                        "iNotificationFTPEnabled": 0,
                        "iNotificationIO1Enabled": 0,
                        "iNotificationRecord1Enabled": 0,
                        "iVideoInputChannelID": 0,
                        "id": 1,
                        "sEventType": "VRI"
                      }
                    )"_json;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    } else if (path_specific_resource.find("motion-detection/") !=
               std::string::npos) {

      if (!path_channel_resource.compare("0")) {
        // // set
        // dbserver_event_set((char *)TABLE_MOVE_DETECTION, config, 0);
        // // get new info
        // str = dbserver_event_get((char *)TABLE_MOVE_DETECTION);
        // content = nlohmann::json::parse(str).at("jData").at(0);
        // move_detect_item_set(content, 0);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    } else if (path_specific_resource.find("regional-invasion/") !=
               std::string::npos) {

      if (!path_channel_resource.compare("0")) {
        // set
        int value_int;
        nlohmann::json param = event_config;
        if (param.dump().find("iEnabled") != param.dump().npos) {
          value_int = atoi(param.at("iEnabled").dump().c_str());
          hon_event_ri_set_enabled(value_int);
        }
        if (param.dump().find("iPositionX") != param.dump().npos) {
          value_int = atoi(param.at("iPositionX").dump().c_str());
          hon_event_ri_set_position_x(value_int);
        }
        if (param.dump().find("iPositionY") != param.dump().npos) {
          value_int = atoi(param.at("iPositionY").dump().c_str());
          hon_event_ri_set_position_y(value_int);
        }
        if (param.dump().find("iHeight") != param.dump().npos) {
          value_int = atoi(param.at("iHeight").dump().c_str());
          hon_event_ri_set_height(value_int);
        }
        if (param.dump().find("iWidth") != param.dump().npos) {
          value_int = atoi(param.at("iWidth").dump().c_str());
          hon_event_ri_set_width(value_int);
        }
        if (param.dump().find("iProportion") != param.dump().npos) {
          value_int = atoi(param.at("iProportion").dump().c_str());
          hon_event_ri_set_proportion(value_int);
        }
        if (param.dump().find("iSensitivityLevel") != param.dump().npos) {
          value_int = atoi(param.at("iSensitivityLevel").dump().c_str());
          hon_event_ri_set_sensitivity_level(value_int);
        }
        if (param.dump().find("iTimeThreshold") != param.dump().npos) {
          value_int = atoi(param.at("iTimeThreshold").dump().c_str());
          hon_event_ri_set_time_threshold(value_int);
        }
        // get new info
        content = regional_invasion_get();
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (path_specific_resource.find("schedules") !=
                 std::string::npos) {

        if (!path_channel_resource.empty()) {
          // set
          int para_id = GetScheduleId(path_channel_resource);
          if (para_id >= 0) {
            // dbserver_event_set((char *)TABLE_EVENT_SCHEDULES, config,
            // para_id);
            // mediaserver_sync_schedules();
            // str = dbserver_event_get((char *)TABLE_EVENT_SCHEDULES);
            // content = nlohmann::json::parse(str).at("jData").at(para_id);
            // content.erase("id");
            content = R"(
                        {
                          "sSchedulesJson": "[[],[],[],[],[],[],[]]"
                        }
                      )"_json;
            Resp.setHeader(HttpStatus::kOk, "OK");
            Resp.setApiData(content);
          } else {
            Resp.setErrorResponse(HttpStatus::kNotImplemented,
                                  "Not Implemented");
          }
        } else {
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        }
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    } else if (!path_specific_resource.compare("take-photo")) {
      hon_take_photo();
      Resp.setHeader(HttpStatus::kOk, "OK");
    }
    #endif
      if (!path_specific_resource.compare("start-record")) {
      hon_storage_record_start();
      Resp.setHeader(HttpStatus::kOk, "OK");
    } else if (!path_specific_resource.compare("stop-record")) {
      hon_storage_record_stop();
      Resp.setHeader(HttpStatus::kOk, "OK");
    } else {
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}



} // namespace cgi
} // namespace rockchip
