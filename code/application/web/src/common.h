// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __CGI_COMMON_API_H__
#define __CGI_COMMON_API_H__

#include "api_handler.h"
#include "jwt-cpp/jwt.h"
#include <cstring>
#ifdef USE_RKIPC
#include "ipcweb_client/client.h"
#else
#include <dbserver.h>
#include <mediaserver.h>
#include <netserver.h>
#include <storage_manager.h>
#include <system_manager.h>
#endif
#include <fstream>
#include <time.h>
#include "openssl/rand.h"
#define SECRET "rockchip"
#define EXPIRE_SECONDS 900// 15 minutes
#define MAX_ATTEMPTS 5
#define LOCKOUT_ENDTIME 900//15 minutes
#define API_FORBIDDEN 403
#define API_UNAUTHORIZED 401
// #define API_SUCCESS 0
// #define API_FAILURE 1
// #define API_INAVLID_ARGUMENT 2
// #define API_SQL_DB_FAILURE 3
// #define API_MEM_ALLOC_FAILURE 4

namespace rockchip {
namespace cgi {

std::string ipv4_address_get();
std::string current_time_get();
time_t standard_to_timestamp(std::string standard_time);
std::string generate_session_id(const std::string& ip, const std::string& username, const std::string& expiry_token);
std::vector<unsigned char> jwt_token_get(std::string session_id, std::string ip_address,int auth, long expiretime,std::string username);
int jwt_token_verify(std::vector<unsigned char> encrypted_token, std::string session_id, HttpRequest &Req);
unsigned char FromHex(unsigned char x);
std::string DecodeURI(std::string &str);
bool isIPLockedOut(const std::string& ipAddress);
std::pair<bool, int>  InsertUpdateIP(const std::string& ipAddress);
std::vector<unsigned char> aes_encrypt(const std::string& plaintext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv);
std::string aes_decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv,bool return_hex_flag);
std::string base64_encode(const std::vector<unsigned char>& data);
std::vector<unsigned char> base64_decode(const std::string& encoded);
std::string print_hex(const std::vector<unsigned char>& data);
struct SessionData {
    std::string session_id;
     std::vector<unsigned char> session_key;
    std::vector<unsigned char> initialization_vector;
    SessionData(const std::string& id)
        : session_id(id), session_key(32), initialization_vector(16) {
        // Generate random key and IV
        RAND_bytes(session_key.data(), session_key.size());
        RAND_bytes(initialization_vector.data(), initialization_vector.size());
    }
};
void convert_space_in_param(std::string &param);
void getStringFromErrorCode(RetCode ret_code, std::string &error_string);

} // namespace cgi
} // namespace rockchip

#endif // __CGI_COMMON_API_H__
