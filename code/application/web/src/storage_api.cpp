// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "storage_api.h"
#include "common.h"
#include <time.h>
#include <unistd.h>

namespace rockchip {
namespace cgi {



nlohmann::json get_snap_plan_0() {
  nlohmann::json param;
  char *tmp = new char[20];
  int value = 0;

  param.emplace("id", 0);
  param.emplace("iShotNumber", 1);
  param.emplace("sImageType", "JPEG");

  hon_video_get_enable_cycle_snapshot(&value);
  param.emplace("iEnabled", value);
  hon_video_get_image_quality(&value);
  param.emplace("iImageQuality", value);
  hon_video_get_snapshot_interval_ms(&value);
  param.emplace("iShotInterval", value);

  hon_video_get_jpeg_resolution(&tmp);
  param.emplace("sResolution", tmp);
  delete[] tmp;

  return param;
}

int set_snap_plan_0(nlohmann::json param) {
  int value_int;
  std::string value;

  if (param.dump().find("iEnabled") != param.dump().npos) {
    value_int = atoi(param.at("iEnabled").dump().c_str());
    hon_video_set_enable_cycle_snapshot(value_int);
  }
  if (param.dump().find("iImageQuality") != param.dump().npos) {
    value_int = atoi(param.at("iImageQuality").dump().c_str());
    hon_video_set_image_quality(value_int);
  }
  if (param.dump().find("iShotInterval") != param.dump().npos) {
    value_int = atoi(param.at("iShotInterval").dump().c_str());
    hon_video_set_snapshot_interval_ms(value_int);
  }
  if (param.dump().find("sResolution") != param.dump().npos) {
    value = param.at("sResolution").dump();
    value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"
    hon_video_set_jpeg_resolution(value.c_str());
  }

  return 0;
}

void StorageApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  int hdd_id = 3;
  std::string path_api_resource;
  std::string path_specific_resource;
  std::string path_hdd_id;
  nlohmann::json content;

  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_specific_resource.find_first_of("/");
    if (pos_first != -1)
      path_hdd_id = path_specific_resource.substr(
          pos_first + 1, path_specific_resource.size() + 1);
  }

  if (Req.Method == "GET") {
    if (path_specific_resource.find("hdd-list") != std::string::npos) {
      if (path_hdd_id.empty()) { /* path is storage/hdd-list */
        content = R"(
      [{
	"iFormatProg": 0,
	"iFormatStatus": 0,
	"iFreeSize": 0,
	"iMediaSize": 0,
	"iTotalSize": 0,
	"id": 1,
	"sDev": "",
	"sFormatErr": "",
	"sMountPath": "/mnt/sdcard",
	"sName": "SD Card",
	"sStatus": "unmounted",
	"sType": ""
}, {
	"iFormatProg": 0,
	"iFormatStatus": 0,
	"iFreeSize": 12.0438613891602,
	"iMediaSize": 60972,
	"iTotalSize": 12.1327171325684,
	"id": 3,
	"sAttributes": "rw",
	"sDev": "/dev/block/by-name/media",
	"sFormatErr": "",
	"sMountPath": "/userdata/media",
	"sName": "Emmc",
	"sStatus": "mounted",
	"sType": "ext2"
}, {
	"iFormatProg": 0,
	"iFormatStatus": 0,
	"iFreeSize": 0,
	"iMediaSize": 0,
	"iTotalSize": 0,
	"id": 2,
	"sDev": "",
	"sFormatErr": "",
	"sMountPath": "/media/usb0",
	"sName": "U Disk",
	"sStatus": "unmounted",
	"sType": ""
}]
    )"_json;
      } else { /* path example is storage/hdd-list/1 */
        // hdd_id = atoi(path_hdd_id.substr(0, path_hdd_id.size()).c_str());
        // nlohmann::json hdd_list = storage_hdd_list_get();
        // content = hdd_list.at(hdd_id - 1);
      }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "OK");
    } else if(path_specific_resource.find("sdcard-info") != std::string::npos){
        float total_size = 0.0f;
        float free_size = 0.0f;
        int mount_status = 0;
        int free_size_mb = 0;
        int recording_channel = -1;
        char *recording_type = nullptr;
        int fifo_status = 0;
    
        // Call the hon_storage_get_sdcard_size function to get SD card size details
        hon_storage_get_recording_channel(&recording_channel);
        hon_storage_get_recording_type(&recording_type);
        hon_storage_get_FIFO_status(&fifo_status);
        int ret = hon_storage_get_sdcard_size(&total_size, &free_size, &mount_status, &free_size_mb);
        if (ret != 0 || recording_type == nullptr) {
            // LOG_ERROR("Failed to get SD card size\n");
            Resp.setErrorResponse(HttpStatus::kInternalServerError, "Failed to get SD card information");
        } else {
            // Create a JSON object using nlohmann::json
            nlohmann::json content;
            content["iFreeSize"] = std::round(free_size * 10) / 10.0; // Round to 1 decimal place
            content["iTotalSize"] = std::round(total_size * 10) / 10.0; // Round to 1 decimal place
            // Add the mount status to the response
            std::string mount_status_str = (mount_status == 4) ? "mounted" : "unmounted";
            content["sMountStatus"] = mount_status_str;
            // Add the recording channel to the response
            content["iRecordingChannel"] = recording_channel;
            // //if recording channel is -1 then recording status in Stopped else it is recording
            // if (recording_channel == -1) {
            //     content["sRecordingStatus"] = "Stopped";
            // } else {
            //     content["sRecordingStatus"] = "Recording";
            // }
            // Add the recording type to the response
            content["sRecordingType"] = recording_type;
            // Add the FIFO status to the response
            content["iFIFOStatus"] = fifo_status;
            // Add the free size in MB to the response
            content["iFreeSizeMB"] = free_size_mb;
            // Set the response with the SD card size details
            Resp.setHeader(HttpStatus::kOk, "OK");
            Resp.setApiData(HttpStatus::kOk, content, "OK");
        }

        if (recording_type) {
          free(recording_type);
        } 
      
    } else if (path_specific_resource.find("quota") != std::string::npos) {
      // if (path_hdd_id.empty()) { /* path is storage/quota */
      //   // Can only get current storage device quota, here only for test
      //   for (int i = 0; i < storage_hdd_list_get().size(); i++) {
      //     content.push_back(storage_quota_get(i + 1));
      //   }
      // } else { /* path example is storage/quota/1 */
      //   hdd_id = atoi(path_hdd_id.substr(0, path_hdd_id.size()).c_str());
      //   content = storage_quota_get(hdd_id);
      // }
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "OK");
    } else if (path_specific_resource.find("snap-plan") != std::string::npos) {
      if (path_hdd_id.empty()) { /* path is storage/snap-plan */
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      } else {
        hdd_id = atoi(path_hdd_id.substr(0, path_hdd_id.size()).c_str());
        if (hdd_id == 0) { /* path is storage/snap-plan/0 */
          content = get_snap_plan_0();
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "OK");
        } else if (hdd_id == 1) { /* path is storage/snap-plan/1 */
          content = R"(
            {
                "iEnabled": 0,
                "iImageQuality": 90,
                "iShotInterval": 1000,
                "iShotNumber": 4,
                "id": 1,
                "sImageType": "JPEG",
                "sResolution": "1920*1080"
            }
          )"_json;
          Resp.setApiData(HttpStatus::kOk, content, "OK");
        } else {
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        }
      }
    } else if (!path_specific_resource.compare("current-path")) {
      // /* path is storage/current-path */
      // char *str = storage_manager_get_media_path();
      // std::string path = nlohmann::json::parse(str).at("sMountPath");
      // content.emplace("sMountPath", path);
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "OK");
    } else if (path_specific_resource.find("advance-para") !=
               std::string::npos) {
      if (path_hdd_id.empty()) { /* path is storage/advance-para */
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      } else {
        hdd_id =
            atoi((char *)path_hdd_id.substr(0, path_hdd_id.size()).c_str());
        // char *str = dbserver_storage_get((char *)TABLE_STORAGE_ADVANCE_PARA);
        if (!hdd_id) {
          content = R"(
            {
              "iEnabled": 0,
              "id": 0
            }
          )"_json;
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "OK");
        } else {
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        }
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    int video_quota_ratio;
    int picture_quota_ratio;
    int camera_id = 0;
    nlohmann::json diff;
    nlohmann::json cfg_old;
    nlohmann::json storage_config = Req.PostObject; /* must be json::object */
    if (path_specific_resource.find("quota") != std::string::npos) {
      if (path_hdd_id.empty()) { /* path is storage/quota */
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      } else { /* path example is storage/quota/1 */
        // nlohmann::json hdd_list = storage_hdd_list_get();
        // hdd_id = atoi(path_hdd_id.substr(0, path_hdd_id.size()).c_str());
        // std::string path_name = get_hdd_name_by_id(hdd_list, hdd_id);
        // minilog_debug("quota path_name is %s\n", (char *)path_name.c_str());
        // /* Set */
        // if (!path_name.empty())
        //   dbserver_update_storage_config_mountpath((char
        //   *)path_name.c_str());
        // else if (hdd_id == 1)
        //   dbserver_update_storage_config_mountpath((char *)"/mnt/sdcard");
        // else if (hdd_id == 2)
        //   dbserver_update_storage_config_mountpath((char *)"/media/usb0");
        // else if (hdd_id == 3)
        //   dbserver_update_storage_config_mountpath((char
        //   *)"/userdata/media");

        // if (!storage_config.empty()) {
        //   video_quota_ratio =
        //       atoi(storage_config.at("iVideoQuotaRatio").dump().c_str());
        //   picture_quota_ratio =
        //       atoi(storage_config.at("iPictureQuotaRatio").dump().c_str());
        //   dbserver_update_storage_media_folder_duty(camera_id, 0,
        //                                             video_quota_ratio, -1);
        //   dbserver_update_storage_media_folder_duty(camera_id, 1,
        //                                             picture_quota_ratio, -1);
        // }
        // /* Get new info */
        // content = storage_quota_get(hdd_id);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(HttpStatus::kOk, content, "OK");
      }
    } else if (path_specific_resource.find("format") != std::string::npos) {
        int ret = hon_storage_format_sd_card(1);
        if (ret != 0) {
          content = R"(
            {
              "iFormat": 0
            }
          )"_json;
          Resp.setHeader(HttpStatus::kInternalServerError, "Failed to format SD card");
          Resp.setApiData(HttpStatus::kInternalServerError, content, "Failed to format SD card");
        } else {
          content = R"(
            {
              "iFormat": 1
            }
          )"_json;
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "SD card Format success");
        }
    }
    //this endpoint is resposnibble to process the uploaded file from the client( for example: firmaware upgrade fie)
    // else if (path_specific_resource.find("upload") != std::string::npos) {
    //   // Get the file name from the headers or a default name
    //   std::string file_name = Req.GetHeader("X-File-Name");
    //   if (file_name.empty()) {
    //       file_name = "uploaded_file.zip"; // Default file name if none is provided
    //   }
  
    //   // Save the file to the desired location
    //   std::ofstream out_file("/userdata/uploads/" + file_name, std::ios::binary);
    //   if (out_file) {
    //       out_file.write(Req.Body.c_str(), Req.Body.size()); // Write the binary data
    //       out_file.close();
    //       Resp.setHeader(HttpStatus::kOk, "OK");
    //       Resp.setApiData(HttpStatus::kOk, content, "File uploaded successfully");
    //   } else {
    //       Resp.setHeader(HttpStatus::kInternalServerError, "Failed to save file");
    //       Resp.setApiData(HttpStatus::kInternalServerError, content, "Failed to save file");
    //   }
    // }
     else if (path_specific_resource.find("recording-settings") != std::string::npos){
        std::string recording_type = storage_config.at("sRecordingType");
        int recording_channel = -1;
        int FIFO_Status = storage_config.at("iFIFOStatus");
        int free_size_mb = storage_config.at("iFreeSizeMB");
        int req_recording_channel = storage_config.at("iRecordingChannel");
        int ret = 0;
        hon_storage_set_free_size_mb(free_size_mb);
        hon_storage_set_FIFO_status(FIFO_Status);
        hon_storage_get_recording_channel(&recording_channel);
        ret = hon_storage_set_recording_type(recording_type.c_str());
        if (recording_channel != req_recording_channel) {
          ret |= hon_storage_set_recording_channel(req_recording_channel);
        }
       
        if (ret != 0) {
          content = R"(
            {
              "iRecordingType": 0
            }
          )"_json;
          Resp.setHeader(HttpStatus::kInternalServerError, "Failed to set recording type");
          Resp.setApiData(HttpStatus::kInternalServerError, content, "Failed to set recording type");
        } else {
          content.emplace("sRecordingType", recording_type);
          content.emplace("iRecordingChannel", req_recording_channel);
          content.emplace("iFIFOStatus", FIFO_Status);
          content.emplace("iFreeSizeMB", free_size_mb);
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "Set recording type success");
        }
    }
    else if (!path_specific_resource.compare("search")) {
      // if (!storage_config.empty())
      //   content = search_file_list_get(storage_config);
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "OK");
    } else if (path_specific_resource.find("snap-plan") != std::string::npos) {
      if (path_hdd_id.empty()) { /* path is storage/snap-plan */
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      } else {
        hdd_id = atoi(path_hdd_id.substr(0, path_hdd_id.size()).c_str());
        if (hdd_id == 0) { /* path is storage/snap-plan/0 */

          /* Erase unchanged data */
          cfg_old = get_snap_plan_0();
          diff = nlohmann::json::diff(cfg_old, storage_config);
          for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
            if (diff.dump().find(x.key()) == diff.dump().npos)
              storage_config.erase(x.key());
          }
          /* Set */
          if (!storage_config.empty())
            set_snap_plan_0(storage_config);
          /* Get new info */
          content = get_snap_plan_0();
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "OK");
        } else if (hdd_id == 1) { /* path is storage/snap-plan/1 */
          // /* Erase unchanged data */
          // cfg_old = storage_plan_get(1);
          // diff = nlohmann::json::diff(cfg_old, storage_config);
          // for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
          //   if (diff.dump().find(x.key()) == diff.dump().npos)
          //     storage_config.erase(x.key());
          // }
          // /* Set */
          // if (!storage_config.empty())
          //   dbserver_set_storage_plan_snap(
          //       (char *)storage_config.dump().c_str(), 1);
          // /* Get new info */
          // content = storage_plan_get(1);
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "OK");
        } else {
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        }
      }
    } else if (path_specific_resource.find("advance-para") !=
               std::string::npos) {

      if (path_hdd_id.empty()) { /* path is storage/advance-para */
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      } else {
        hdd_id = atoi(path_hdd_id.substr(0, path_hdd_id.size()).c_str());
        if (!hdd_id) {
          // char *str = dbserver_storage_get((char
          // *)TABLE_STORAGE_ADVANCE_PARA);
          // nlohmann::json old_config =
          //     nlohmann::json::parse(str).at("jData").at(hdd_id);
          // nlohmann::json diff =
          //     nlohmann::json::diff(old_config, storage_config);
          // for (auto &x : nlohmann::json::iterator_wrapper(old_config)) {
          //   if (diff.dump().find(x.key()) == diff.dump().npos)
          //     storage_config.erase(x.key());
          // }
          // if (!storage_config.empty()) {
          //   dbserver_storage_set((char *)TABLE_STORAGE_ADVANCE_PARA,
          //                        (char *)storage_config.dump().c_str(),
          //                        hdd_id);
          //   mediaserver_sync_schedules();
          // }
          // str = dbserver_storage_get((char *)TABLE_STORAGE_ADVANCE_PARA);
          // content = nlohmann::json::parse(str).at("jData").at(hdd_id);
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(HttpStatus::kOk, content, "OK");
        } else {
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        }
      }
    } else if (path_specific_resource.find("delete") != std::string::npos) {

      // nlohmann::json file_list = storage_config.at("name");
      // std::string delete_type = storage_config.at("type");
      // int rst = 1;
      // if (!file_list.empty()) {
      //   rst = media_file_delete(delete_type, file_list);
      // }
      // content.emplace("rst", rst);
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(HttpStatus::kOk, content, "OK");
    } else {
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchip
