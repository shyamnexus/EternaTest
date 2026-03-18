// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "system_api.h"
#include "common.h"
#include <fstream>
#include <string.h>
#include <sys/statfs.h>
#include "Logger.h"
#include "http_response.h"
#include <sys/time.h>

#ifdef MEDIASERVER_ROCKFACE
#include <mediaserver.h>
#include <storage_manager.h>
#endif

//************************************ User Account Setting start ************************************
#define		MAX_WEB_SESSION						10
#define		MAX_USER_NAME_LEN					15
#define		MAX_USER_PASSWORD_LEN				64
#define   	MAX_SECURITY_ANS_LEN      			15
#define   	MIN_SECURITY_ANS_LEN      			3
#define 	SECURITY_QUES_LEN 					5
#define		MIN_USER_NAME_LEN					5
#define		MIN_USER_PASSWORD_LEN				45
#define		MAX_USER_ACCOUNT					10
#define   	SESSION_ID_LENGTH         			64

namespace rockchip {
namespace cgi {

std::vector<unsigned char> key(32); // AES-256 key size is 32 bytes
std::string base64_key ="QFQNP1QwiMJ17jLWqroPFThqPE0S9b7vCiIxS7cdM+8=";	
std::string decrypted_pass;
std::string decode_decrypt_password(std::string password)
{
	// Logger& logger = Logger::getInstance();
	// Find the position of the dollar sign
	size_t dollarPos = password.find('$');
	std::string base64_pass;
	std::string aes_iv;
	if (dollarPos != std::string::npos) {
		// Extract the IV, which is the part after the dollar sign
	aes_iv = password.substr(dollarPos + 1); // Get everything after the dollar sign
		base64_pass = password.substr(0, dollarPos);
	}
	// logger.log(Logger::INFO,"\n============aes_iv= ============\n",aes_iv);
	// logger.log(Logger::INFO,"\n============enc_pass= ============\n",base64_pass);
	// std::vector<unsigned char> iv(16);  // AES block size is 16 bytes

	// // Generate random key and IV
	// RAND_bytes(key.data(), key.size());
	// RAND_bytes(iv.data(), iv.size());
				
	
	//  std::string key_hex = print_hex(key);

				
	// logger.log(Logger::INFO,"\n============random key ============\n",key_hex);
	
	// std::string iv_hex = print_hex(iv);

				
	// logger.log(Logger::INFO,"\n============random iv ============\n",iv_hex);
	//  std::string base64_key = base64_encode(key);
	// std::string base64_iv = base64_encode(iv);
	std::vector<unsigned char> decoded_key = base64_decode(base64_key);
	std::vector<unsigned char> decoded_iv = base64_decode(aes_iv);
	std::vector<unsigned char> decoded_pass = base64_decode(base64_pass);
	// logger.log(Logger::INFO,"\n============base64_key  ============\n",base64_key);
	// logger.log(Logger::INFO,"\n============base64_iv iv ============\n",base64_iv);
	// std::vector<unsigned char> encrypted_pass = aes_encrypt(decoded_pass,decoded_key, decoded_iv);
	
	// std::string encrypted_pass_hex = print_hex(encrypted_pass);

				
	// logger.log(Logger::INFO,"\n============encrypted password from request============\n",encrypted_pass_hex);
	// std::string base64_pass = base64_encode(encrypted_pass);
	// logger.log(Logger::INFO,"\n============base64_pass  ============\n",base64_pass);

	// Decode the Base64 encoded key and IV

	// std::string decoded_key_str = print_hex(decoded_key);
	// logger.log(Logger::INFO,"\n============decoded_key_str  ============\n",decoded_key_str);
	// std::string decoded_iv_str = print_hex(decoded_iv);
	// logger.log(Logger::INFO,"\n============decoded_iv_str  ============\n",decoded_iv_str);
	// std::string decoded_pass_str = print_hex(decoded_pass);
	// logger.log(Logger::INFO,"\n============decoded_pass_str  ============\n",decoded_pass_str);
	

		std::string decrypted_pass = aes_decrypt(decoded_pass, decoded_key, decoded_iv,0);
		return decrypted_pass;
					
}
std::string create_fireware_location() {
	int file_id;
	int exist = 1;
	char *str;
	std::ofstream file;
	// Make sure files are not duplicated
	srand((int)time(NULL));
	while (exist) 
	{
		file_id = rand() % 1000;
		file.open("/data/" + file_id);
		if (!file) // open fail
			exist = 0;
		file.close();
	}
	// Create new file
	std::string file_name = "/data/" + std::to_string(file_id);
	std::ofstream new_file(file_name.c_str());
	new_file.close();
	// Get ip address and location
	std::string ipv4_address = ipv4_address_get();
	std::string location = "http://" + ipv4_address +
												 "/cgi-bin/entry.cgi/system/firmware-upgrade?id=" +
												 std::to_string(file_id);

	return location;
}

nlohmann::json device_info_get() {
	nlohmann::json device_info;
	nlohmann::json tmp_json;
	char *tmp = new char[20];

	hon_system_get_deivce_name(&tmp);
	tmp_json.emplace("id", 0);
	tmp_json.emplace("name", "deviceName");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "false");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_telecontrol_id(&tmp);
	tmp_json.emplace("id", 1);
	tmp_json.emplace("name", "telecontrolID");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "false");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_model(&tmp);
	tmp_json.emplace("id", 2);
	tmp_json.emplace("name", "model");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_serial_number(&tmp);
	tmp_json.emplace("id", 3);
	tmp_json.emplace("name", "serialNumber");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_firmware_version(&tmp);
	tmp_json.emplace("id", 4);
	tmp_json.emplace("name", "firmwareVersion");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_encoder_version(&tmp);
	tmp_json.emplace("id", 5);
	tmp_json.emplace("name", "encoderVersion");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_web_version(&tmp);
	tmp_json.emplace("id", 6);
	tmp_json.emplace("name", "webVersion");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_plugin_version(&tmp);
	tmp_json.emplace("id", 7);
	tmp_json.emplace("name", "pluginVersion");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_channels_number(&tmp);
	tmp_json.emplace("id", 8);
	tmp_json.emplace("name", "channelsNumber");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_hard_disks_number(&tmp);
	tmp_json.emplace("id", 9);
	tmp_json.emplace("name", "hardDisksNumber");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_alarm_inputs_number(&tmp);
	tmp_json.emplace("id", 10);
	tmp_json.emplace("name", "alarmInputsNumber");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_alarm_outputs_number(&tmp);
	tmp_json.emplace("id", 11);
	tmp_json.emplace("name", "alarmOutputsNumber");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_firmware_version_info(&tmp);
	tmp_json.emplace("id", 12);
	tmp_json.emplace("name", "firmwareVersionInfo");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_manufacturer(&tmp);
	tmp_json.emplace("id", 13);
	tmp_json.emplace("name", "manufacturer");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	tmp_json.clear();
	hon_system_get_hardware_id(&tmp);
	tmp_json.emplace("id", 14);
	tmp_json.emplace("name", "hardwareId");
	tmp_json.emplace("value", tmp);
	tmp_json.emplace("ro", "true");
	device_info.push_back(tmp_json);

	delete[] tmp;

	return device_info;
}

int device_info_set(nlohmann::json param) {
	std::string value = param.at("value").dump().c_str();
	value.erase(0, 1).erase(value.end() - 1, value.end()); // erase \"

	if (!param.at("name").dump().compare("\"deviceName\"")) {
		hon_system_set_deivce_name(value.c_str());
	} else if (!param.at("name").dump().compare("\"telecontrolID\"")) {
		hon_system_set_telecontrol_id(value.c_str());
	} else {
		minilog_debug("device_info_set, no support set %s\n",
			param.at("name").dump().c_str());
	}

	return 0;
}

nlohmann::json get_user_info() {
	char *tmp = new char[1000];
	int ret = 0;
	nlohmann::json user_info = nlohmann::json::array();
	nlohmann::json tmp_json;
	char sql_query[] = "SELECT * FROM users;";

	ret = hon_system_get_user_details(tmp,sql_query);
	if (ret!=0)
	{	
		if (tmp) {
			delete[] tmp;
		}
		return user_info;
	}
	tmp_json = nlohmann::json::parse(tmp);   
	for (int i = 0; i < tmp_json["username"].size(); i++) 
	{
		nlohmann::json dump_module;  
		dump_module["sUserName"] = tmp_json["username"].at(i);
		dump_module["iUserLevel"] =std::stoi(tmp_json["userlevel"].at(i).get<std::string>());
		user_info.push_back(dump_module);
	}
	if (tmp) {
		delete[] tmp;
	}  
	return user_info;
}

nlohmann::json is_register_user(nlohmann::json put_info) {

	std::string user_name = put_info.at("sUserName");
	std::string password = put_info.at("sPassword");
	char *db_password = new char[MAX_USER_PASSWORD_LEN];
	std::string db_password_string;
	int userlevel = 0;
	nlohmann::json res;
	// Logger& logger = Logger::getInstance();

	// logger.log(Logger::INFO,"\n============in function is_register_user============\n");
	// logger.log(Logger::INFO,"\n============in function is_register_user-->user_name.c_str()============\n",user_name.c_str());
	// logger.log(Logger::INFO,"\n============in function is_register_user -->password============\n",password.c_str()); 

	if(0 == hon_system_check_user_exists(user_name.c_str(),db_password,&userlevel,true))
	{
		// logger.log(Logger::INFO,"\n============in function is_register_user-->userlevel============\n",userlevel);
		db_password_string.assign(db_password);
		// logger.log(Logger::INFO,"\n============in function is_register_user -->database password============\n",db_password_string.c_str());
		if(std::strcmp(password.c_str(),db_password_string.c_str()) == 0)
		{
			// logger.log(Logger::INFO,"\n============in function is_register_user -->password============\n",password.c_str());
			// logger.log(Logger::INFO,"\n============User Found in is_register_user============\n",userlevel);
			res.emplace("authlevel", userlevel);
		}

	}
	// logger.log(Logger::INFO,"\n============in function is_register_user return============\n",res);
	return res;
}



nlohmann::json GetPara(std::string para_name) {
	nlohmann::json content;
	if (!para_name.compare("screenshot")) {
		content =
				"{\"dynamic\":{\"id\":{\"0\":{\"iShotInterval\":{\"for\":\"timing\","
				"\"range\":{\"max\":604800000,\"min\":1000},\"type\":\"range\"},"
				"\"timeUnit\":{\"for\":\"timing\",\"options\":[\"seconds\",\"minutes\","
				"\"hours\",\"days\"],\"type\":\"options\"}},\"1\":{\"iShotInterval\":{"
				"\"for\":\"timing\",\"range\":{\"max\":65535,\"min\":1000},\"type\":"
				"\"range\"},\"timeUnit\":{\"for\":\"timing\",\"options\":[\"seconds\","
				"\"minutes\"],\"type\":\"options\"}}}},\"relation\":{\"iImageQuality\":"
				"{\"50\":\"low\",\"70\":\"middle\",\"90\":\"high\"}},\"static\":{"
				"\"iImageQuality\":{\"options\":[50,70,90],\"type\":\"options\"},"
				"\"iShotNumber\":{\"range\":{\"max\":120,\"min\":1},\"type\":\"range\"}"
				",\"sImageType\":{\"options\":[\"JPEG\"],\"type\":\"options\"},"
				"\"sResolution\":{\"refer\":[4,\"para\",\"dynamic\",\"sStreamType\","
				"\"mainStream\",\"sResolution\"],\"type\":\"refer\"}}}";
	} else if (!para_name.compare("screenshot-schedule")) {
		content = "[{\"color\":\"#87CEEB\",\"name\":\"timing\"}]";
	} else if (!para_name.compare("video-plan-schedule")) {
		content = "[{\"color\":\"#87CEEB\",\"name\":\"timing\"},{\"color\":\"#"
							"74B558\",\"name\":\"motion-detect\"},{\"color\":\"#D71820\","
							"\"name\":\"alarm\"},{\"color\":\"#E58705\",\"name\":"
							"\"motionOrAlarm\"},{\"color\":\"#B9E2FE\",\"name\":"
							"\"motionAndAlarm\"},{\"color\":\"#AA6FFF\",\"name\":\"event\"}]";
	} else if (!para_name.compare("smart-cover")) {
		content = "";
	} else if (!para_name.compare("video-encoder")) {
		char cap[4096];
		hon_system_capability_get_video(cap);
		if (strlen(cap) == 0) {
			content =
					"{\"disabled\":[{\"name\":\"sStreamType\",\"options\":{\"subStream\":"
					"{"
					"\"sOutputDataType\":\"H.264\"}},\"type\":\"disabled/"
					"limit\"},{"
					"\"name\":"
					"\"sRCMode\",\"options\":{\"CBR\":{\"sRCQuality\":null}},\"type\":"
					"\"disabled\"},{\"name\":"
					"\"unspport\","
					"\"options\":{\"sSVC\":null}"
					",\"type\":\"disabled\"}],\"dynamic\":{\"sStreamType\":{\"mainStream\":{\"iMaxRate\":{\"options\":[256,512,"
					"1024,2048,3072,4096,6144,8192,12288,16384],\"type\":\"options\"},"
					"\"sResolution\":{\"options\":[\"2560*1520\",\"1280*720\",\"640*"
					"480\"],"
					"\"type\":\"options\"}}}},\"layout\":{\"encoder\":[\"sStreamType\","
					"\"sResolution\",\"sRCMode\",\"sRCQuality\","
					"\"sFrameRate\",\"sOutputDataType\","
					"\"sSVC\",\"iMaxRate\",\"iGOP\"]},"
					"\"static\":{\"iGOP\":{\"range\":{\"max\":400,\"min\":0},\"type\":"
					"\"range\"},\"sFrameRate\":{\"dynamicRange\":{"
					"\"maxRate\":1},\"options\":[\"1/16\",\"1/"
					"8\",\"1/4\",\"1/"
					"2\",\"1\",\"2\",\"4\",\"6\",\"8\",\"10\",\"12\",\"14\",\"16\","
					"\"18\","
					"\"20\",\"25\",\"30\"],\"type\":\"options/"
					"dynamicRange\"},\"sOutputDataType\":{"
					"\"options\":["
					"\"H.264\",\"H.265\"],\"type\":\"options\"},\"sRCMode\":{\"options\":"
					"["
					"\"CBR\",\"VBR\"],\"type\":\"options\"},\"sRCQuality\":{\"options\":["
					"\"lowest\",\"lower\",\"low\",\"medium\",\"high\",\"higher\","
					"\"highest\"],\"type\":\"options\"},\"sSVC\":{\"options\":[\"open\","
					"\"close\"],\"type\":\"options\"},\"sStreamType\":{\"options\":["
					"\"mainStream\",\"subStream\",\"thirdStream\"],\"type\":\"options\"}}}";
		} else {

			content = nlohmann::json::parse(cap);
		}
	} else if (!para_name.compare("isp")) {
		char cap[4096];
		hon_system_capability_get_image_adjustment(cap);
		content.emplace("image_adjustment", nlohmann::json::parse(cap));
		hon_system_capability_get_image_blc(cap);
		content.emplace("image_blc", nlohmann::json::parse(cap));
		hon_system_capability_get_image_enhancement(cap);
		content.emplace("image_enhancement", nlohmann::json::parse(cap));
		hon_system_capability_get_image_exposure(cap);
		content.emplace("image_exposure", nlohmann::json::parse(cap));
		hon_system_capability_get_image_night_to_day(cap);
		content.emplace("image_night_to_day", nlohmann::json::parse(cap));
		hon_system_capability_get_image_video_adjustment(cap);
		content.emplace("image_video_adjustment", nlohmann::json::parse(cap));
		hon_system_capability_get_image_white_blance(cap);
		content.emplace("image_white_blance", nlohmann::json::parse(cap));
		if (strlen(cap) == 0) {
			content = R"(
				{
					"image_adjustment":
					"{\"layout\":{\"image_adjustment\":[\"iBrightness\",\"iContrast\",\"iSaturation\",\"iSharpness\",\"iHue\"]},\"static\":{\"iBrightness\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iContrast\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iHue\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iSaturation\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iSharpness\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}}",
					"image_blc":
					"{\"disabled\":[{\"name\":\"sHLC\",\"options\":{\"open\":{\"sBLCRegion\":null}},\"type\":\"disabled\"},{\"name\":\"sBLCRegion\",\"options\":{\"open\":{\"iDarkBoostLevel\":null,\"iHLCLevel\":null,\"sHLC\":null}},\"type\":\"disabled\"}],\"dynamic\":{\"sBLCRegion\":{\"open\":{\"iBLCStrength\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}},\"sHDR\":{\"HDR2\":{\"iHDRLevel\":{\"options\":[1,2,3,4,5],\"type\":\"options\"}},\"close\":{\"sBLCRegion\":{\"options\":[\"close\",\"open\"],\"type\":\"options\"},\"sHLC\":{\"options\":[\"close\",\"open\"],\"type\":\"options\"}}},\"sHLC\":{\"open\":{\"iDarkBoostLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iHLCLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}},\"sWDR\":{\"open\":{\"iWDRLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}}},\"layout\":{\"image_blc\":[\"sHDR\",\"iHDRLevel\",\"sBLCRegion\",\"iBLCStrength\",\"sHLC\",\"iHLCLevel\",\"iDarkBoostLevel\"]},\"static\":{\"sHDR\":{\"options\":[\"close\",\"HDR2\"],\"type\":\"options\"}}}",
					"image_enhancement":
					"{\"dynamic\":{\"sDehaze\":{\"open\":{\"iDehazeLevel\":{\"range\":{\"max\":10,\"min\":0,\"step\":1},\"type\":\"range\"}}},\"sDistortionCorrection\":{\"FEC\":{\"iFecLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}},\"LDCH\":{\"iLdchLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}},\"sNoiseReduceMode\":{\"2dnr\":{\"iSpatialDenoiseLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}},\"3dnr\":{\"iTemporalDenoiseLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}},\"mixnr\":{\"iSpatialDenoiseLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iTemporalDenoiseLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}}},\"layout\":{\"image_enhancement\":[\"sNoiseReduceMode\",\"iSpatialDenoiseLevel\",\"iTemporalDenoiseLevel\",\"sDehaze\",\"iDehazeLevel\",\"sGrayScaleMode\",\"sDistortionCorrection\",\"iLdchLevel\",\"iFecLevel\",\"iImageRotation\"]},\"static\":{\"iImageRotation\":{\"options\":[0,90,270],\"type\":\"options\"},\"sDIS\":{\"options\":[\"open\",\"close\"],\"type\":\"options\"},\"sDehaze\":{\"options\":[\"open\",\"close\",\"auto\"],\"type\":\"options\"},\"sDistortionCorrection\":{\"options\":[\"LDCH\",\"close\"],\"type\":\"options\"},\"sFEC\":{\"options\":[\"open\",\"close\"],\"type\":\"options\"},\"sGrayScaleMode\":{\"options\":[\"[0-255]\"],\"type\":\"options\"},\"sNoiseReduceMode\":{\"options\":[\"close\",\"2dnr\",\"3dnr\",\"mixnr\"],\"type\":\"options\"}}}",
					"image_exposure":
					"{\"dynamic\":{\"sExposureMode\":{\"auto\":{\"iAutoIrisLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}},\"manual\":{\"sExposureTime\":{\"options\":[\"1\",\"1/3\",\"1/6\",\"1/12\",\"1/25\",\"1/50\",\"1/100\",\"1/150\",\"1/200\",\"1/250\",\"1/500\",\"1/750\",\"1/1000\",\"1/2000\",\"1/4000\",\"1/10000\",\"1/100000\"],\"type\":\"options\"},\"sGainMode\":{\"options\":[\"auto\",\"manual\"],\"type\":\"options\"}}},\"sGainMode\":{\"manual\":{\"iExposureGain\":{\"range\":{\"max\":100,\"min\":1,\"step\":1},\"type\":\"range\"}}}},\"layout\":{\"image_exposure\":[\"sExposureMode\",\"sExposureTime\",\"sGainMode\",\"iExposureGain\"]},\"static\":{\"sExposureMode\":{\"options\":[\"auto\",\"manual\"],\"type\":\"options\"}}}",
					"image_night_to_day":
					"{\"disabled\":[{\"name\":\"sNightToDay\",\"options\":{\"day\":{\"iLightBrightness\":null,\"sFillLightMode\":null},\"night\":{\"iDarkBoostLevel\":null,\"iHDRLevel\":null,\"iHLCLevel\":null,\"sHDR\":null,\"sHLC\":\"close\"}},\"type\":\"disabled\"}],\"dynamic\":{\"sNightToDay\":{\"auto\":{\"iNightToDayFilterLevel\":{\"options\":[0,1,2,3,4,5,6,7],\"type\":\"options\"},\"iNightToDayFilterTime\":{\"range\":{\"max\":10,\"min\":3,\"step\":1},\"type\":\"range\"}},\"schedule\":{\"sDawnTime\":{\"input\":\"time\",\"type\":\"input\"},\"sDuskTime\":{\"input\":\"time\",\"type\":\"input\"}}},\"sOverexposeSuppress\":{\"open\":{\"sOverexposeSuppressType\":{\"options\":[\"auto\",\"manual\"],\"type\":\"options\"}}},\"sOverexposeSuppressType\":{\"manual\":{\"iDistanceLevel\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}}},\"layout\":{\"image_night_to_day\":[\"sNightToDay\",\"iNightToDayFilterLevel\",\"iNightToDayFilterTime\",\"sDawnTime\",\"sDuskTime\",\"sFillLightMode\",\"iLightBrightness\"]},\"static\":{\"iLightBrightness\":{\"range\":{\"max\":100,\"min\":0,\"step\":10},\"type\":\"range\"},\"sNightToDay\":{\"options\":[\"day\",\"night\"],\"type\":\"options\"},\"sFillLightMode\":{\"type\":\"options\",\"options\":[\"IR\"]}}}",
					"image_video_adjustment":
					"{\"layout\":{\"image_video_adjustment\":[\"sPowerLineFrequencyMode\",\"sImageFlip\"]},\"static\":{\"sImageFlip\":{\"options\":[\"close\",\"flip\",\"mirror\",\"centrosymmetric\"],\"type\":\"options\"},\"sPowerLineFrequencyMode\":{\"options\":[\"PAL(50HZ)\",\"NTSC(60HZ)\", \"AUTO\"],\"type\":\"options\"},\"sSceneMode\":{\"options\":[\"indoor\",\"outdoor\"],\"type\":\"options\"}}}",
					"image_white_blance":
					"{\"dynamic\":{\"White Balance Style\":{\"Manual WhiteBalance\":{\"iWhiteBalanceBlue\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iWhiteBalanceGreen\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"},\"iWhiteBalanceRed\":{\"range\":{\"max\":100,\"min\":0,\"step\":1},\"type\":\"range\"}}}},\"layout\":{\"image_white_blance\":[\"White Balance Style\",\"iWhiteBalanceRed\",\"iWhiteBalanceGreen\",\"iWhiteBalanceBlue\"]},\"static\":{\"White Balance Style\":{\"options\":[\"Manual WhiteBalance\",\"Auto WhiteBalance\",\"Incandescent\",\"Fluorescent Lamp\",\"Daylight\",\"Cloudy Daylight\", \"Twilight\", \"Shade\"],\"type\":\"options\"}}}"
				}
			)"_json;
		}  
	} else {
		content =
				"{\"auth\":4,\"item\":[{\"auth\":4,\"name\":\"preview\"},{\"auth\":-1,"
				"\"item\":[{\"auth\":4,\"item\":[{\"auth\":0,\"name\":\"delete\"}],"
				"\"name\":\"videoRecord\"},{\"auth\":4,\"item\":[{\"auth\":0,\"name\":"
				"\"delete\"}],\"name\":\"pictureRecord\"}],\"name\":\"download\"},{"
				"\"auth\":-1,\"item\":[{\"auth\":4,\"item\":[{\"auth\":4,\"item\":[{"
				"\"auth\":1,\"name\":\"modify\"}],\"name\":\"ListManagement\"},{"
				"\"auth\":1,\"name\":\"AddOne\"},{\"auth\":1,\"name\":\"BatchInput\"}],"
				"\"name\":\"MemberList\"},{\"auth\":-1,\"item\":[{\"auth\":4,\"item\":["
				"{\"auth\":0,\"name\":\"modify\"}],\"name\":\"SnapShot\"}],\"name\":"
				"\"SnapShot\"},{\"auth\":-1,\"item\":[{\"auth\":4,\"item\":[{\"auth\":"
				"0,\"name\":\"modify\"}],\"name\":\"Control\"}],\"name\":\"Control\"},{"
				"\"auth\":1,\"item\":[{\"auth\":1,\"name\":\"ParaConfig\"}],\"name\":"
				"\"Config\"}],\"name\":\"face\"},{\"auth\":-1,\"item\":[{\"auth\":1,"
				"\"item\":[{\"auth\":1,\"name\":\"FacePara\"},{\"auth\":1,\"name\":"
				"\"ROI\"}],\"name\":\"Config\"}],\"name\":\"face-para\"},{\"auth\":-1,"
				"\"item\":[{\"auth\":4,\"item\":[{\"auth\":4,\"item\":[{\"auth\":1,"
				"\"name\":\"modify\"}],\"name\":\"MemberList\"},{\"auth\":1,\"name\":"
				"\"AddOne\"},{\"auth\":1,\"name\":\"BatchInput\"},{\"auth\":4,\"item\":"
				"[{\"auth\":0,\"name\":\"modify\"}],\"name\":\"SnapShot\"},{\"auth\":4,"
				"\"item\":[{\"auth\":0,\"name\":\"modify\"}],\"name\":\"Control\"}],"
				"\"name\":\"Manage\"}],\"name\":\"face-manage\"},{\"auth\":1,\"item\":["
				"{\"auth\":1,\"item\":[{\"auth\":1,\"item\":[{\"auth\":1,\"name\":"
				"\"basic\"},{\"auth\":1,\"name\":\"time\"}],\"name\":\"Settings\"},{"
				"\"auth\":1,\"item\":[{\"auth\":1,\"name\":\"upgrade\"},{\"auth\":-1,"
				"\"name\":\"log\"}],\"name\":\"Maintain\"},{\"auth\":-1,\"item\":[{"
				"\"auth\":-1,\"name\":\"authentication\"},{\"auth\":-1,\"name\":"
				"\"ipAddrFilter\"},{\"auth\":-1,\"name\":\"securityService\"}],"
				"\"name\":\"Security\"},{\"auth\":0,\"name\":\"User\"}],\"name\":"
				"\"System\"},{\"auth\":1,\"item\":[{\"auth\":1,\"item\":[{\"auth\":1,"
				"\"name\":\"TCPIP\"},{\"auth\":-1,\"name\":\"DDNS\"},{\"auth\":-1,"
				"\"name\":\"PPPoE\"},{\"auth\":1,\"name\":\"Port\"},{\"auth\":-1,"
				"\"name\":\"uPnP\"}],\"name\":\"Basic\"},{\"auth\":1,\"item\":[{"
				"\"auth\":1,\"name\":\"Wi-Fi\"},{\"auth\":-1,\"name\":\"SMTP\"},{"
				"\"auth\":-1,\"name\":\"FTP\"},{\"auth\":-1,\"name\":\"eMail\"},{"
				"\"auth\":-1,\"name\":\"Cloud\"},{\"auth\":-1,\"name\":\"Protocol\"},{"
				"\"auth\":-1,\"name\":\"QoS\"},{\"auth\":-1,\"name\":\"Https\"}],"
				"\"name\":\"Advanced\"}],\"name\":\"Network\"},{\"auth\":1,\"item\":[{"
				"\"auth\":1,\"name\":\"Encoder\"},{\"auth\":1,\"name\":"
				"\"AdvancedEncoder\"},{\"auth\":1,\"name\":\"ROI\"},{\"auth\":1,"
				"\"name\":\"RegionCrop\"}],\"name\":\"Video\"},{\"auth\":1,\"item\":[{"
				"\"auth\":1,\"name\":\"AudioParam\"}],\"name\":\"Audio\"},{\"auth\":1,"
				"\"item\":[{\"auth\":1,\"name\":\"DisplaySettings\"},{\"auth\":1,"
				"\"name\":\"OSDSettings\"},{\"auth\":1,\"name\":\"PrivacyCover\"},{"
				"\"auth\":1,\"name\":\"PictureMask\"}],\"name\":\"Image\"},{\"auth\":1,"
				"\"item\":[{\"auth\":1,\"name\":\"MotionDetect\"},{\"auth\":1,\"name\":"
				"\"IntrusionDetection\"},{\"auth\":-1,\"name\":\"AlarmInput\"},{"
				"\"auth\":-1,\"name\":\"AlarmOutput\"},{\"auth\":-1,\"name\":"
				"\"Abnormal\"}],\"name\":\"Event\"},{\"auth\":1,\"item\":[{\"auth\":1,"
				"\"item\":[{\"auth\":1,\"name\":"
				"\"ScreenshotPlan\"},{\"auth\":1,\"name\":\"ScreenshotPara\"}],"
				"\"name\":\"PlanSettings\"},{\"auth\":1,\"item\":[{\"auth\":1,\"name\":"
				"\"HardDiskManagement\"},{\"auth\":-1,\"name\":\"NAS\"},{\"auth\":-1,"
				"\"name\":\"CloudStorage\"}],\"name\":\"StorageManage\"}],\"name\":"
				"\"Storage\"},{\"auth\":1,\"item\":[{\"auth\":1,\"name\":\"MarkCover\"}"
				",{\"auth\":-1,\"name\":\"MaskArea\"},{\"auth\":-1,\"name\":"
				"\"RuleSettings\"},{\"auth\":-1,\"name\":\"AdvancedCFG\"}],\"name\":"
				"\"Intel\"},{\"auth\":-1,\"item\":[{\"auth\":-1,\"name\":"
				"\"GateConfig\"},{\"auth\":-1,\"name\":\"ScreenConfig\"}],\"name\":"
				"\"Peripherals\"}],\"name\":\"config\"},{\"auth\":4,\"name\":\"about\"}"
				"],\"name\":\"header\"}";
	}
	return content;
}

void SystemApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
	char *str;
	std::string path_api_resource;
	std::string path_specific_resource;
	std::string para_channel;
	nlohmann::json content = nlohmann::json({});

	/* Get Path Information */
	int pos_first = Req.PathInfo.find_first_of("/");
	path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
	pos_first = path_api_resource.find_first_of("/");
	if (pos_first != -1) 
	{
		path_specific_resource = path_api_resource.substr(pos_first + 1, path_api_resource.size());
		pos_first = path_specific_resource.find_first_of("/");
		if (pos_first != -1)
			para_channel = path_specific_resource.substr(
					pos_first + 1, path_specific_resource.size() + 1);
	}

	if (Req.Method == "GET") 
	{
		if (!path_specific_resource.compare("device-info")) {
			content = device_info_get();
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk,content,"OK");
		} else if (!path_specific_resource.compare("remain-space")) {
			struct statfs diskInfo;
			statfs("/userdata/", &diskInfo);
			// Free space for non-root users
			unsigned long long availableDisk = diskInfo.f_bavail * diskInfo.f_bsize;
			content.emplace("availableDisk", availableDisk);
			Resp.setHeader(HttpStatus::kOk, "OK");
			Resp.setApiData(HttpStatus::kOk,content,"OK");
		}
		
		else if (path_specific_resource.find("para") != std::string::npos) {
			if (para_channel.empty()) {
				Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
				Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
			} else {
				content = GetPara(para_channel);
				if (!content.empty()) {
					Resp.setHeader(HttpStatus::kOk, "OK");
					 Resp.setApiData(HttpStatus::kOk,content,"OK");
				} else {
					Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
					Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
				}
			}
		} 
		else if (!path_specific_resource.compare("firmware-upload")) {
			// Temporary firmware upload implementation
			// Upload binary file and store in /mnt/sd, then reboot device
			if (Req.Files.empty()) {
				Resp.setApiData(HttpStatus::kBadRequest, content, "No file uploaded");
				Resp.setHeader(HttpStatus::kBadRequest, "No file uploaded");
				return;
			}

			// Get the uploaded file
			const auto& uploadedFile = Req.Files.at(0);
			const std::string firmwarePath = "/mnt/sd/firmware_update.bin";
			
			try {
				// Write the uploaded binary file to /mnt/sd
				std::ofstream outFile(firmwarePath, std::ios::binary | std::ios::out);
				if (!outFile) {
					Resp.setApiData(HttpStatus::kInternalServerError, content, "Failed to create firmware file");
					Resp.setHeader(HttpStatus::kInternalServerError, "Failed to create firmware file");
					return;
				}
				
				outFile.write(uploadedFile.getData().c_str(), uploadedFile.getDataLength());
				outFile.close();
				
				if (outFile.fail()) {
					Resp.setApiData(HttpStatus::kInternalServerError, content, "Failed to write firmware file");
					Resp.setHeader(HttpStatus::kInternalServerError, "Failed to write firmware file");
					return;
				}
				
				// Return success response
				content.emplace("message", "Firmware uploaded successfully. Device will reboot.");
				content.emplace("filepath", firmwarePath);
				Resp.setApiData(HttpStatus::kOk, content, "Firmware uploaded successfully");
				Resp.setHeader(HttpStatus::kOk, "OK");
				
				// Trigger reboot after response is sent
				// Using system call to schedule reboot
				system("sync && reboot &");
				
			} catch (const std::exception& e) {
				content.emplace("error", e.what());
				Resp.setApiData(HttpStatus::kInternalServerError, content, "Firmware upload failed");
				Resp.setHeader(HttpStatus::kInternalServerError, "Firmware upload failed");
				return;
			}
		}
		else if (path_specific_resource.find("login") != std::string::npos) 
		{ 
			if (!para_channel.compare("users"))
			{
				content = get_user_info();
				if (!content.empty()) 
				{
					Resp.setHeader(HttpStatus::kOk, "OK");
					Resp.setApiData(HttpStatus::kOk,content,"Users retrieved successfully");
					return;
				}
				else
				{
					Resp.setHeader(HttpStatus::kBadRequest, "No Users found");
					Resp.setApiData(HttpStatus::kBadRequest,content,"No Users found");
					return;
				}
			}
			else 
			{
				Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
				Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
				return;
			}
		} 
		else 
		{
			Resp.setHeader(HttpStatus::kNotImplemented, "Not Implemented");
			Resp.setApiData(HttpStatus::kNotImplemented,content,"Not Implemented");
			return;
		}
	} 
	else if ((Req.Method == "POST") || (Req.Method == "PUT")) 
	{
		nlohmann::json system_config = Req.PostObject; /* must be json::object */
		/*
		* Author: Suryawanshi Neelam
		* Date: 2025-04-22
		* 
		* Replace the default_password with a hashed version of the plaintext password (default_password_hash).
		* The same changes are applicable in C source files.Store the passwords in OPTEE
		*/
		std::string default_password_hash = "e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7";
		std::string default_password = "Admin@123";
		#ifdef TEST
		if (!path_specific_resource.compare("device-info")) {

			if (!Req.PostObject.at("ro").dump().compare("\"true\"")) {
				Resp.setErrorResponse(HttpStatus::kBadRequest, "Value is read-only!");
			} else {
				/* Set */
				device_info_set(Req.PostObject);
				/* Get new info */
				// content = device_info_get();
				Resp.setHeader(HttpStatus::kOk, "OK");
				Resp.setApiData(content);
			}
		} else if (!path_specific_resource.compare("firmware-upgrade")) {
			// int total_length = 0;
			int transmitted_length;
			int begin_position;
			int end_position;
			int current_transfer_length;
			std::string progress_rate = "";Data
			// for (auto p : Req.Params) {
			//   if (p.Key == "total-length")
			//     total_length = atoi(p.Value.c_str());
			// }
			for (auto p : Req.Params) {
				if ((p.Key == "upload-type") && (p.Value == "resumable")) {
					// Create a transmission resource
					Resp.setHeader(HttpStatus::kOk, "OK");
					Resp.addHeader("X-Location", create_fireware_location());
				} else if (p.Key == "id") {
					std::ofstream file;
					std::string file_id = p.Value;

					file.open("/data/" + file_id, std::ofstream::app); // append write
					if (!file) {
						Resp.setErrorResponse(HttpStatus::kNotFound, "Not Found");
					} else if (Req.ContentLength == 0) {
						// Query the current progress
						file.seekp(0, std::ofstream::end);
						transmitted_length = file.tellp();
						progress_rate = "bytes 0-" + std::to_string(transmitted_length - 1);
						Resp.setHeader(HttpStatus::kResumeIncomplete, "Resume Incomplete");
						content.emplace("range", progress_rate);
						Resp.setApiData(content);
					} else {
						// Transfer
						begin_position = file.tellp();
						file << Req.PostData;
						end_position = file.tellp();
						current_transfer_length = end_position - begin_position;
						// transmitted_length is not necessarily equal to end_position,
						// it is possible to change a paragraph in the middle of the file
						file.seekp(0, std::ofstream::end);
						transmitted_length = file.tellp();
						if (Req.ContentLength != current_transfer_length) {
							// Request for retransmission due to Datatransmission error
							progress_rate = "bytes 0-" + std::to_string(begin_position - 1);
							Resp.setHeader(HttpStatus::kResumeIncomplete,
														 "Resume Incomplete");
						} else {
							progress_rate =
									"bytes 0-" + std::to_string(transmitted_length - 1);
							if (Req.ContentLength < 512 * 1024) {
								// Transfer completed
								Resp.setHeader(HttpStatus::kCreated, "Created");
							} else if (Req.ContentLength == 512 * 1024) {
								Resp.setHeader(HttpStatus::kResumeIncomplete,
															 "Resume Incomplete");
							} else {
								Resp.setHeader(HttpStatus::kResumeIncomplete,
															 "More than 512KB!");
							}
						}
						content.emplace("range", progress_rate);
						Resp.setApiData(content);
					}
					file.close();
				} else if (p.Key == "start") {
					int fireware_id = stoi(p.Value);
					std::string path = "/userdata/" + std::to_string(fireware_id);
					hon_system_upgrade(path.c_str());
					Resp.setHeader(HttpStatus::kOk, "OK");
				}
			}
		} else if (!path_specific_resource.compare("reboot")) {
			hon_system_reboot();
		} else if (!path_specific_resource.compare("pre-factory-reset")) {
			//       // for face db
			// #ifdef MEDIASERVER_ROCKFACE
			//       char *str = storage_manager_get_media_path();
			//       std::string mount_path =
			//       nlohmann::json::parse(str).at("sMountPath");
			//       minilog_debug("format when reset, path is %s\n", (char
			//       *)mount_path.c_str());
			//       storage_manager_diskformat((char *)mount_path.c_str(), (char
			//       *)"fat32");
			//       mediaserver_clear_face_db();
			// #else
			//       dbserver_face_reset((char *)TABLE_FACE_LIST);
			// #endif
		} else if (!path_specific_resource.compare("factory-reset")) {
			hon_system_factory_reset();
		} else if (!path_specific_resource.compare("export-log")) {
			const char *path = (const char *)"/userdata/export.log";
			hon_system_export_log(path);
			std::string ipv4_address = ipv4_address_get();
			std::string location = "http://" + ipv4_address + path;
			content.emplace("location", location);
			Resp.setApiData(content);
		} else if (!path_specific_resource.compare("export-db")) {
			const char *path = (const char *)"/userdata/export.ini";
			hon_system_export_db(path);
			std::string ipv4_address = ipv4_address_get();
			std::string location = "http://" + ipv4_address + path;
			content.emplace("location", location);
			Resp.setApiData(content);
		} else if (!path_specific_resource.compare("import-db")) {
			if (Req.Params.empty()) {
				int end_position = 0;
				// overwrite
				std::ofstream db_file("/userdata/import.ini", std::ofstream::out);
				db_file << Req.Files.at(0).getData();
				end_position = db_file.tellp();
				db_file.close();
				if (end_position == Req.Files.at(0).getDataLength()) {
					Resp.setHeader(HttpStatus::kOk, "OK");
				} else {
					Resp.setErrorResponse(HttpStatus::kBadRequest, "db upload failed!");
				}
			}
			for (auto p : Req.Params) {
				if (p.Key == "start")
					hon_system_import_db((const char *)"/userdata/import.ini");
			}
		} else
		#endif
		if (path_specific_resource.find("login") != std::string::npos) 
		{		
		// Logger& logger = Logger::getInstance();
			if (para_channel.empty()) 
			{ /* path is login */
				

				//check if account is locked
				if(isIPLockedOut(Req.RemoteAddress) == true)
				{
					// logger.log(Logger::INFO,"\n============The device is locked and the lock time is 15 minutes; please try again later.============\n");
					Resp.setApiData(HttpStatus::kLocked, content, "The device is locked and the lock time is 15 minutes. Please try again later.");
					Resp.setHeader(HttpStatus::kLocked, "The device is locked and the lock time is 15 minutes. Please try again later.");
					return;
				}
				char sql_query[256] = {0};
				int session_count = 0;
				std::string username = system_config.at("sUserName");
				std::string password = system_config.at("sPassword");
				//  logger.log(Logger::INFO,"\n============path is login============\n");
				//  logger.log(Logger::INFO,"\n============username  is notempty============\n",username);
				//  logger.log(Logger::INFO,"\n============password is notempty============\n",password);
				
				   
				if ((!username.empty()) && (!password.empty())) 
				{
				// logger.log(Logger::INFO,"\n============username and password is notempty============\n");
					if (username.length() >= MIN_USER_NAME_LEN &&
							username.length() <= MAX_USER_NAME_LEN &&
							password.length() >= MIN_USER_PASSWORD_LEN &&
							password.length() <= MAX_USER_PASSWORD_LEN) 
					{
						// logger.log(Logger::INFO,"\n============username and password are of valid length============\n");
						memset(sql_query, '\0', 1);
						snprintf(sql_query, sizeof(sql_query), "SELECT COUNT(*) FROM session_history;");
						sql_query[strlen(sql_query)] = '\0';
						int ret = hon_count_db_records(&session_count, sql_query);
						// logger.log(Logger::INFO,"\n============hon_count_db_records ret = ============\n",ret);
						if (ret!=0) 
						{
							Resp.setApiData(HttpStatus::kBadRequest, content, "Internal Server Error login!!");
							Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error login!!");
							return;
						}
						// logger.log(Logger::INFO,"\n============session_count= ============\n",session_count);
						if (session_count >= MAX_WEB_SESSION) {
							Resp.setHeader(HttpStatus::kRateLimiting, "Maximum session limit reached. Please try again later.");
							Resp.setApiData(HttpStatus::kRateLimiting, content,"Maximum session limit reached. Please try again later.");
							return;
						}
						// Find the position of the dollar sign
						// size_t dollarPos = password.find('$');
						// std::string base64_pass;
						// std::string aes_iv;
						// if (dollarPos != std::string::npos) {
						// 	// Extract the IV, which is the part after the dollar sign
						// aes_iv = password.substr(dollarPos + 1); // Get everything after the dollar sign
						//  base64_pass = password.substr(0, dollarPos);
						// }
						// logger.log(Logger::INFO,"\n============aes_iv= ============\n",aes_iv);
						// logger.log(Logger::INFO,"\n============enc_pass= ============\n",base64_pass);
						// std::vector<unsigned char> iv(16);  // AES block size is 16 bytes

						// // Generate random key and IV
						// RAND_bytes(key.data(), key.size());
						// RAND_bytes(iv.data(), iv.size());
									
						
						//  std::string key_hex = print_hex(key);

									
						// logger.log(Logger::INFO,"\n============random key ============\n",key_hex);
						
						// std::string iv_hex = print_hex(iv);

									
						// logger.log(Logger::INFO,"\n============random iv ============\n",iv_hex);
						//  std::string base64_key = base64_encode(key);
    					// std::string base64_iv = base64_encode(iv);
						// std::vector<unsigned char> decoded_key = base64_decode(base64_key);
						// std::vector<unsigned char> decoded_iv = base64_decode(aes_iv);
						// std::vector<unsigned char> decoded_pass = base64_decode(base64_pass);
						// logger.log(Logger::INFO,"\n============base64_key  ============\n",base64_key);
						// logger.log(Logger::INFO,"\n============base64_iv iv ============\n",base64_iv);
						// std::vector<unsigned char> encrypted_pass = aes_encrypt(decoded_pass,decoded_key, decoded_iv);
						
						// std::string encrypted_pass_hex = print_hex(encrypted_pass);

									
						// logger.log(Logger::INFO,"\n============encrypted password from request============\n",encrypted_pass_hex);
						// std::string base64_pass = base64_encode(encrypted_pass);
						// logger.log(Logger::INFO,"\n============base64_pass  ============\n",base64_pass);

						// Decode the Base64 encoded key and IV

						// std::string decoded_key_str = print_hex(decoded_key);
						// logger.log(Logger::INFO,"\n============decoded_key_str  ============\n",decoded_key_str);
						// std::string decoded_iv_str = print_hex(decoded_iv);
						// logger.log(Logger::INFO,"\n============decoded_iv_str  ============\n",decoded_iv_str);
						// std::string decoded_pass_str = print_hex(decoded_pass);
						// logger.log(Logger::INFO,"\n============decoded_pass_str  ============\n",decoded_pass_str);
						

						decrypted_pass = decode_decrypt_password(password);
						
						// logger.log(Logger::INFO,"\n============decrypted_pass =   ============\n",decrypted_pass);
			
						system_config["sPassword"] = decrypted_pass;
						content = is_register_user(system_config); 
						// content.emplace("sDecryptedPassword", decrypted_password);  
						// logger.log(Logger::INFO,"\n============after is_register_user= ============\n",content);
						if(!content.empty())
						{
							// logger.log(Logger::INFO,"\n============username and password found============\n");
							int auth = content.at("authlevel");
							Resp.setHeader(HttpStatus::kOk, "OK");
							//set the time if the year is 2021
							auto now = std::chrono::system_clock::now();
							std::time_t current_time = std::chrono::system_clock::to_time_t(now);
							std::tm *time_info = std::localtime(&current_time);
							int current_year = 1900 + time_info->tm_year;
							// std::string device_time;
							time_t stamp_time;
							if(current_year <= 2024)
							{
								std::string time_s = system_config.at("cDateTime");
								// logger.log(Logger::INFO,"\n============set date time============\n");
								 stamp_time = standard_to_timestamp(time_s);
								struct timeval tv;
								tv.tv_sec = stamp_time;
								tv.tv_usec = 0;
								int ret = settimeofday(&tv, NULL);
								 if (ret != 0) {
									Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to Set the time in login");
									Resp.setHeader(HttpStatus::kBadRequest, "Failed to Set the time in login");
									return;
								}
				
								system("hwclock -w -u");
							}
							// Specify the timezone
								// std::string timezone = "Asia/Kolkata"; // This can be dynamically set based on the request

								// // Set the timezone using setenv and tzset
								// if (setenv("TZ", timezone.c_str(), 1) != 0) {
								// 	Resp.setApiData(HttpStatus::kBadRequest, content, "Failed to Set the timezone in login");
								// 	Resp.setHeader(HttpStatus::kBadRequest, "Failed to Set the timezone in login");
								// 	return;
								// }
							// 	tzset(); // Update the timezone settings
							// 	// Optionally, print the local time to verify
							// 	std::tm *local_time = std::localtime(&stamp_time);
							// 	std::ostringstream oss_time;
    						// 	oss_time << std::put_time(local_time, "%Y-%m-%d %H:%M:%S"); // Format as needed
							// 	device_time = oss_time.str();
							// 	system("hwclock -w -u");
							// content.emplace("device_time", device_time);
			
							long expire_time = EXPIRE_SECONDS;
							auto expire_at = std::chrono::system_clock::now() + std::chrono::seconds{expire_time};
						 	auto expire_at_sec = std::chrono::duration_cast<std::chrono::seconds>(expire_at.time_since_epoch()).count();
							// logger.log(Logger::INFO,"\n============login token duration============\n",duration);
							// logger.log(Logger::INFO,"\n============login token expire_at============\n",seconds_since_epoch);
						 	std::string expiry_token = std::to_string(expire_at_sec); 
							std::string session_id = generate_session_id(Req.RemoteAddress, username, expiry_token);
							std::vector<unsigned char> encrypted_token = jwt_token_get(session_id, Req.RemoteAddress, auth,expire_at_sec,username);
							if (encrypted_token.size() > 0)
							{
								std::ostringstream oss;
								for (unsigned char c : encrypted_token) {
										oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
								}
								std::string encrypted_token_hex = oss.str();

								// Append the session_id to the encrypted_token_hex
								std::string final_token = encrypted_token_hex + session_id;
								// logger.log(Logger::INFO,"\n============encrypted token in login============\n",final_token);
								// save the jwt token 
								content.emplace("token", final_token);
								//  memset(sql_query, '\0', 1);
								// snprintf(sql_query, sizeof(sql_query), "INSERT INTO session_history (session_id, ip_address,auth,expire_time) VALUES ('%s','%s','%d','%lld');", session_id.c_str(),Req.RemoteAddress.c_str(),auth,expire_at_sec);
								// content.emplace("expireat", seconds_since_epoch);
								// logger.log(Logger::INFO,"\n============User logged in succesfully============\n");
								// if (0 == hon_system_manage_user_record(sql_query)) {
								char sql_query[256] = {0};
								// snprintf(sql_query, sizeof(sql_query), "SELECT * FROM IPInfo WHERE ipAddress = '%s' LIMIT 1;", Req.RemoteAddress.c_str());
								// if (0 == hon_system_record_exists(sql_query)) {
								// 	Resp.setApiData(HttpStatus::kOk, content,"User logged in succesfully");
								// 	return;
								// }
								snprintf(sql_query, sizeof(sql_query), "DELETE FROM IPInfo WHERE ipAddress = '%s';", Req.RemoteAddress.c_str());
								hon_system_manage_user_record(sql_query);
								// logger.log(Logger::INFO,"\n============User logged in succesfully============\n");
								// if (0 == hon_system_manage_user_record(sql_query)) {
								Resp.setApiData(HttpStatus::kOk, content,"User logged in succesfully");
								return;
								// }
								// else {
								// 	Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error!!!");
								// 	Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error!!!");
								// 	return;
								// }
								
								// }
							}
						 	else
							{
								// logger.log(Logger::INFO,"\n===========Internal Server Error!!.===========\n"); 
								Resp.setApiData(HttpStatus::kBadRequest, content, "Internal Server Error token!!");
								Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error token!!");
								return;
							}
						}
						else
						{
							// logger.log(Logger::INFO,"\n===========Invalid username or password, before InsertUpdateIP ===========\n"); 
							auto result = InsertUpdateIP(Req.RemoteAddress);
							if(result.first == true)
							{
								int attempts = result.second;
    							std::string responseMessage = "Incorrect username or password. The system will be locked for 15 minutes after " + std::to_string(MAX_ATTEMPTS - attempts) + " continuous wrong password attempts.";

								// logger.log(Logger::INFO,"\n===========Invalid username or password.===========\n",responseMessage); 
								Resp.setHeader(HttpStatus::kUnauthorized, responseMessage.c_str());
								Resp.setApiData(HttpStatus::kUnauthorized, content,responseMessage.c_str());
								return;
							}
							else{
								// logger.log(Logger::INFO,"\n===========Internal Server Error.===========\n");
								Resp.setHeader(HttpStatus::kUnauthorized, "Internal Server Error for insert IP");
								Resp.setApiData(HttpStatus::kUnauthorized, content,"Internal Server Error for insert IP");
								return;
							}
						}	
					}
					else 
					{
						// logger.log(Logger::INFO,"\n===========Invalid username or password length.===========\n"); 
						Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid Username or Password length");
						Resp.setHeader(HttpStatus::kBadRequest, "Invalid Username or Password length");
						return;
					}
				}
				else {
					// logger.log(Logger::INFO,"\n===========Please provide the user details===========\n"); 
					Resp.setApiData(HttpStatus::kBadRequest, content, "Please provide the user details");
					Resp.setHeader(HttpStatus::kBadRequest, "Please provide the user details");
					return;
				}

			} 
			else if (!para_channel.compare("modify")) 
			{ /* path is login/modify */
				int new_userlevel;
				std::string new_username = system_config.at("sUserName");
				std::string new_password = system_config.at("sPassword");
				new_userlevel = system_config.at("iUserLevel");
				if ((!new_username.empty()) && (!new_password.empty())) 
				{
					if (new_username.length() >= MIN_USER_NAME_LEN && 
						new_username.length() <= MAX_USER_NAME_LEN && 
						new_password.length() >= MIN_USER_PASSWORD_LEN && 
						new_password.length() <= MAX_USER_PASSWORD_LEN) {
						if(new_username == "admin")
						{
							if(new_userlevel > 0)
							{
								Resp.setApiData(HttpStatus::kBadRequest, content,"Cannot change authority level for the default user!!");
								Resp.setHeader(HttpStatus::kBadRequest, "Cannot change authority level for the default user!!");
								return;
							}
							decrypted_pass = decode_decrypt_password(new_password);
							if(decrypted_pass == default_password)
							{
								Resp.setApiData(HttpStatus::kBadRequest, content,"Setting a default password is not allowed. Please choose a unique password.");
								Resp.setHeader(HttpStatus::kBadRequest, "Setting a default password is not allowed. Please choose a unique password.");
								return;
							}
						}
						if(hon_system_check_user_exists(new_username.c_str(),NULL,0,false) == 0)
						{
							char sql_query[256] = {0};
							memset(sql_query, '\0', sizeof(sql_query));
							snprintf(sql_query, sizeof(sql_query), "UPDATE users SET password = '%s', userlevel = %d WHERE username = '%s';", decrypted_pass.c_str(),new_userlevel,new_username.c_str());
							if (0 == hon_system_manage_user_record(sql_query)) 
							{
							
								memset(sql_query, '\0', sizeof(sql_query));
								snprintf(sql_query, sizeof(sql_query), "DELETE FROM session_history WHERE username = '%s' AND username IN (SELECT username FROM session_history WHERE username = '%s');",new_username.c_str(),new_username.c_str());
								if (0 == hon_system_manage_user_record(sql_query)) 
								{
									content =  get_user_info();
									Resp.setHeader(HttpStatus::kOk, "OK");
									Resp.setApiData(HttpStatus::kOk, content,"User data modified succesfully");
									return;
								}
								else 
								{
									Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for deleting session history in modify !!!");
									Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for deleting session history in modify !!!");
									return;
								}
							}
							else 
							{
								Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for modify !!!");
								Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for modify!!!");
								return;
							}
						}
						else
						{
							Resp.setApiData(HttpStatus::kBadRequest, content,"User data not found");
							Resp.setHeader(HttpStatus::kBadRequest, "User data not found");
							return;
						}
					}
				 	else {
						// logger.log(Logger::INFO,"\n===========Invalid username or password length.===========\n"); 
						Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid Username or Password length");
						Resp.setHeader(HttpStatus::kBadRequest, "Invalid Username or Password length");
						return;
					} 
				}
				else 
				{
					Resp.setApiData(HttpStatus::kBadRequest, content, "Please provide the user details");
					Resp.setHeader(HttpStatus::kBadRequest, "Please provide the user details");
					return;
				}

		} 
		else if (!para_channel.compare("resetpwd")) 
		{ /* path is login/resetpwd */
		// Logger& logger = Logger::getInstance();
			std::string new_username = system_config.at("sUserName");
			std::string new_password = system_config.at("sPassword");
			
			std::string new_securityque = system_config.at("sSecurityQue");
		
			std::string new_securityans = system_config.at("sSecurityAns");
			// logger.log(Logger::INFO,"\n============in resetpwd function===========\n");
			if((new_username.empty()) || (new_password.empty()) || (new_securityans.empty())|| (new_securityque.empty()))
			{
				// logger.log(Logger::INFO,"\n============Please provide the user details============\n");
				Resp.setApiData(HttpStatus::kBadRequest, content, "Please provide the user details");
				Resp.setHeader(HttpStatus::kBadRequest, "Please provide the user details");
				return;
			}
			else if(new_username == "admin")
			{
				// logger.log(Logger::INFO,"\n============in resetpwd function user is admin===========\n");
				if (new_password.length() < MIN_USER_PASSWORD_LEN || new_password.length() > MAX_USER_PASSWORD_LEN)
				{	
					Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid Password length");
					Resp.setHeader(HttpStatus::kBadRequest, "Invalid  Password length");
					return;
				}
				decrypted_pass = decode_decrypt_password(new_password);
				
				// logger.log(Logger::INFO,"\n============decrypted_pass =   ============\n",decrypted_pass);
				if(decrypted_pass == default_password)
				{
					Resp.setApiData(HttpStatus::kBadRequest, content,"Setting a default password is not allowed. Please choose a unique password.");
					Resp.setHeader(HttpStatus::kBadRequest, "Setting a default password is not allowed. Please choose a unique password.");
					return;
				}
				if(new_securityans.length() < MIN_SECURITY_ANS_LEN || new_securityans.length() > MAX_SECURITY_ANS_LEN ||
					new_securityque.length() != SECURITY_QUES_LEN)
				{
					Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid length of Security Parameters");
					Resp.setHeader(HttpStatus::kBadRequest, "Invalid length of Security Parameters");
					return;
				}	
				char sql_query[1024] = {0};
				memset(sql_query, '\0', 1);
				snprintf(sql_query, sizeof(sql_query), "UPDATE users SET password = '%s', securityans = '%s',securityque = '%s' WHERE username = '%s';", decrypted_pass.c_str(),new_securityans.c_str(), new_securityque.c_str(), new_username.c_str());
				if (0 == hon_system_manage_user_record(sql_query))
				{
					Resp.setHeader(HttpStatus::kOk, "OK");
					Resp.setApiData(HttpStatus::kOk, content,"User data reset succesfully");
					return;
				}
				else 
				{
					Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for resetpwd!!!");
					Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for resetpwd!!!");
					return;
				}
			}
			else
			{
				Resp.setApiData(HttpStatus::kBadRequest, content,"Please provide default user");
				Resp.setHeader(HttpStatus::kBadRequest, "Please provide default user");
				return;
			}
		} 
		else if (!para_channel.compare("forgetpwd")) 
		{
			/* path is login/forgetpwd */
			char sql_query[256] = {0};
			std::string new_username = system_config.at("sUserName");
			std::string new_password = system_config.at("sPassword");
			std::string new_securityque = system_config.at("sSecurityQue");
			std::string new_securityans = system_config.at("sSecurityAns"); 
			if((new_username.empty()) || (new_password.empty()) || (new_securityans.empty()))
			{
				Resp.setApiData(HttpStatus::kBadRequest, content, "Please provide the user details");
				Resp.setHeader(HttpStatus::kBadRequest, "Please provide the user details");
				return;
			}
			else
			{
				if (new_username.length() >= MIN_USER_NAME_LEN && new_username.length() <= MAX_USER_NAME_LEN && 
					new_password.length() >= MIN_USER_PASSWORD_LEN && new_password.length() <= MAX_USER_PASSWORD_LEN)
				{
					decrypted_pass = decode_decrypt_password(new_password);
					if((new_username == "admin") && (decrypted_pass == default_password))
					{	
						Resp.setApiData(HttpStatus::kBadRequest, content,"Setting a default password is not allowed. Please choose a unique password.");
						Resp.setHeader(HttpStatus::kBadRequest, "Setting a default password is not allowed. Please choose a unique password.");
						return;	
					}
					if(new_securityans.length() < MIN_SECURITY_ANS_LEN || new_securityans.length() > MAX_SECURITY_ANS_LEN ||
					new_securityque.length() != SECURITY_QUES_LEN)
					{
						Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid length of Security Parameters");
						Resp.setHeader(HttpStatus::kBadRequest, "Invalid length of Security Parameters");
						return;
					}	
					char* new_password_arr = new char[decrypted_pass.length() + 1];
					std::strcpy(new_password_arr, decrypted_pass.c_str());
					char* new_securityque_arr = new char[new_securityque.length() + 1];
					std::strcpy(new_securityque_arr, new_securityque.c_str());
					char* new_securityans_arr = new char[new_securityans.length() + 1];
					std::strcpy(new_securityans_arr, new_securityans.c_str());
					int ret = hon_system_verify_security_ans(new_username.c_str(),new_password_arr,new_securityans_arr,new_securityque_arr);
					if(ret == 0)
					{
						

						snprintf(sql_query, sizeof(sql_query), "DELETE FROM IPInfo WHERE ipAddress = '%s';", Req.RemoteAddress.c_str());
						hon_system_manage_user_record(sql_query);
						memset(sql_query, '\0', sizeof(sql_query));
						snprintf(sql_query, sizeof(sql_query), "DELETE FROM session_history WHERE username = '%s' AND username IN (SELECT username FROM session_history WHERE username = '%s');",new_username.c_str(),new_username.c_str());
						if (0 == hon_system_manage_user_record(sql_query)) 
						{
							Resp.setHeader(HttpStatus::kOk, "OK");
							Resp.setApiData(HttpStatus::kOk, content,"Password changed succesfully");
							return;
						}
						else 
						{
							Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for session_history forgetpwd");
							Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for session_history forgetpwd");
							return;
						}
					}
					else if(ret == 1)
					{
						Resp.setApiData(HttpStatus::kBadRequest, content,"User data not found");
						Resp.setHeader(HttpStatus::kBadRequest, "User data not found");
						return;
					}
					else if(ret == 2) 
					{
						Resp.setApiData(HttpStatus::kBadRequest, content,"Invalid security answer");
						Resp.setHeader(HttpStatus::kBadRequest, "Invalid security answer");
						return;
					}
					else 
					{
						Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for forgetpwd");
						Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for forgetpwd");
						return;
					}
				}
				else 
				{
					// logger.log(Logger::INFO,"\n===========Invalid username or password length.===========\n"); 
					Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid Username or Password length");
					Resp.setHeader(HttpStatus::kBadRequest, "Invalid Username or Password length");
				}
			}

		} else if (!para_channel.compare("add")) 
		{ /* path is login/add */
			// Logger& logger = Logger::getInstance();
			// logger.log(Logger::INFO,"\n============path is login/add============\n");
			int user_num;
			std::string new_username = system_config.at("sUserName");
			std::string new_password = system_config.at("sPassword");
			int new_userlevel = system_config.at("iUserLevel");
			std::string new_securityque = system_config.at("sSecurityQue");
			std::string new_securityans = system_config.at("sSecurityAns");
			hon_system_get_user_num(&user_num);  
			// logger.log(Logger::INFO,"\n============path is login/add user_num is ===========\n",user_num);
			if (user_num >=MAX_USER_ACCOUNT)
			{     
				Resp.setApiData(HttpStatus::kBadRequest, content,"Exceeded the user limit!!");
				Resp.setHeader(HttpStatus::kBadRequest, "Exceeded the user limit!!"); 
				return;
					
			}
			else  if((new_username.empty()) || (new_password.empty()) || (new_securityans.empty()) || (new_securityque.empty()))
			{
				Resp.setApiData(HttpStatus::kBadRequest, content, "Please provide the user details");
				Resp.setHeader(HttpStatus::kBadRequest, "Please provide the user details");
				return;
			}
			else {
				if (new_username.length() >= MIN_USER_NAME_LEN && new_username.length() <= MAX_USER_NAME_LEN && 
					new_password.length() >= MIN_USER_PASSWORD_LEN && new_password.length() <= MAX_USER_PASSWORD_LEN)
				{
					if(hon_system_check_user_exists(new_username.c_str(),NULL,0,false) == 1) 
					{
						decrypted_pass = decode_decrypt_password(new_password);
						char sql_query[256] = {0};
						memset(sql_query, '\0', 1);
						snprintf(sql_query, sizeof(sql_query), "INSERT INTO users (username, password,userlevel,securityans,securityque) VALUES ('%s', '%s', %d  ,'%s','%s');", new_username.c_str(),decrypted_pass.c_str(),new_userlevel,new_securityans.c_str(),new_securityque.c_str());
						if (0 == hon_system_manage_user_record(sql_query))
						{
							content = get_user_info();
							Resp.setHeader(HttpStatus::kOk, "OK");
							Resp.setApiData(HttpStatus::kOk, content,"User data added succesfully");
						}
						else 
						{
							Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for add user!!!");
							Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for add user!!!");
							return;
						}
					}
					else
					{
						Resp.setApiData(HttpStatus::kBadRequest, content,"User Already Exists!!");
						Resp.setHeader(HttpStatus::kBadRequest, "User Already Exists!!");
					}
				}
				else 
				{
						// logger.log(Logger::INFO,"\n===========Invalid username or password length.===========\n"); 
					Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid Username or Password length");
					Resp.setHeader(HttpStatus::kBadRequest, "Invalid Username or Password length");
				}
			}
		}
		else 
		{
			Resp.setApiData(HttpStatus::kBadRequest, content, "Not Implemented");
			Resp.setHeader(HttpStatus::kBadRequest, "Not Implemented");
		}
	}	 
	else {
			Resp.setApiData(HttpStatus::kBadRequest, content,"Not Implemented");
			Resp.setHeader(HttpStatus::kBadRequest, "Not Implemented");
	}
	} 

	else if (Req.Method == "DELETE") 
	{
		if (path_specific_resource.find("delete") != std::string::npos) 
		{
			// Logger& logger = Logger::getInstance();
 
			if (Req.Params.empty()) 
			{
				Resp.setApiData(HttpStatus::kBadRequest, content,"User Data Empty!!");
				Resp.setHeader(HttpStatus::kBadRequest, "User Data Empty!!");
				return;
			}	
			for (auto p : Req.Params) {
				if (p.Key == "sUserName") {
					std::string delete_username = p.Value;
					if (delete_username == "") 
					{
						Resp.setApiData(HttpStatus::kBadRequest, content,"User Data Empty!!");
						Resp.setHeader(HttpStatus::kBadRequest, "User Data Empty!!");
						return;
					}
					else if (delete_username.length() >= MIN_USER_NAME_LEN && delete_username.length() <= MAX_USER_NAME_LEN)
					{
						
						if(delete_username != "admin")
						{

							if(hon_system_check_user_exists(delete_username.c_str(),NULL,0,false) == 0)
							{
								char sql_query[256] = {0};
								memset(sql_query, '\0', 1);
								snprintf(sql_query, sizeof(sql_query), "DELETE FROM users WHERE username = '%s';", delete_username.c_str());
								if (0 == hon_system_manage_user_record(sql_query)) {
									memset(sql_query, '\0', sizeof(sql_query));
									snprintf(sql_query, sizeof(sql_query), "DELETE FROM session_history WHERE username = '%s' AND username IN (SELECT username FROM session_history WHERE username = '%s');",delete_username.c_str(),delete_username.c_str());
									if (0 == hon_system_manage_user_record(sql_query)) 
									{
										content = get_user_info();
										Resp.setApiData(HttpStatus::kOk, content,"User data deleted succesfully");
										Resp.setHeader(HttpStatus::kOk, "OK");
										return;
									}
									else {
										Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for session_history delete user!!!");
										Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for session_history delete user!!!");
										return;
									}
								}
								else {
									Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for delete user!!!");
									Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for delete user!!!");
									return;
								}
							}
							Resp.setApiData(HttpStatus::kBadRequest, content,"User data not found");
							Resp.setHeader(HttpStatus::kBadRequest, "User data not found");
							return;
						}
						else {
							Resp.setApiData(HttpStatus::kBadRequest, content,"Default User data cannot be deleted");
							Resp.setHeader(HttpStatus::kBadRequest, "Default User data cannot be deleted");
							return;
						}
					}
					else {
						// logger.log(Logger::INFO,"\n===========Invalid username or password length.===========\n"); 
						Resp.setApiData(HttpStatus::kBadRequest, content, "Invalid Username or Password length");
						Resp.setHeader(HttpStatus::kBadRequest, "Invalid Username or Password length");
					}
				}
				else
				{
					Resp.setApiData(HttpStatus::kBadRequest, content,"Bad Request Params");
					Resp.setHeader(HttpStatus::kBadRequest, "Bad Request Params");
					return;
				}
			}
		}
		else if (path_specific_resource.find("logout") != std::string::npos) 
		 { 
			if (Req.Params.empty()) {
				char sql_query[256] = {0};
				memset(sql_query, '\0', 1);
				snprintf(sql_query, sizeof(sql_query), "SELECT * FROM session_history WHERE session_id = '%s' LIMIT 1;", Req.SessionId.c_str());
				if (0 == hon_system_record_exists(sql_query)) {
					Resp.setApiData(HttpStatus::kBadRequest, content,"User Session data not found!!");
					Resp.setHeader(HttpStatus::kBadRequest, "User Session data not found!!");
					return;
				}
				snprintf(sql_query, sizeof(sql_query), "DELETE FROM session_history WHERE session_id = '%s';", Req.SessionId.c_str());
				if (0 == hon_system_manage_user_record(sql_query)) {
					Resp.setApiData(HttpStatus::kOk, content,"User logged out successfully");
					Resp.setHeader(HttpStatus::kOk, "OK");
					return;
				}
				else {
					Resp.setApiData(HttpStatus::kBadRequest, content,"Internal Server Error for logout!!!");
					Resp.setHeader(HttpStatus::kBadRequest, "Internal Server Error for logout!!!");
					return;
				}
			}
			else
			{
				Resp.setApiData(HttpStatus::kBadRequest, content,"Bad Request Params");
				Resp.setHeader(HttpStatus::kBadRequest, "Bad Request Params");
				return;
			}
		 }
		else if (!path_specific_resource.compare("firmware-upgrade")) {
			if (!Req.Params.empty()) {
				for (auto p : Req.Params) {
					if (p.Key == "id") {
						std::string file_id_s = p.Value;
						std::string file_name = "/data/" + file_id_s;
						if (!remove(file_name.c_str()))
							Resp.setHeader(HttpStatus::kOk, "OK");
					}
				}
			} else {
				Resp.setApiData(HttpStatus::kBadRequest, " ","Not Implemented");
				Resp.setHeader(HttpStatus::kBadRequest, "Not Implemented");
			}
		} else {
			 Resp.setApiData(HttpStatus::kBadRequest, " ","Not Implemented in login");
				Resp.setHeader(HttpStatus::kBadRequest, "Not Implemented");
		}
	} else {
		 Resp.setApiData(HttpStatus::kBadRequest, " ","Not Implemented in PUT POST GET DELETE");
		Resp.setHeader(HttpStatus::kBadRequest, "Not Implemented ");
	}

} // namespace cgi
} // namespace rockchip
}
