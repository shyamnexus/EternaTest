// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common.h"
#include "Logger.h"
#include "openssl/sha.h"
#include "openssl/evp.h"
#include <openssl/bio.h>
#include <sqlite3.h>
namespace rockchip {
namespace cgi {

#define ADMIN 0
#define OPERATOR 1
#define USER 2
#define DB_PATH_UM "/usr/share/usermng.db"

// Function to encode data to Base64
std::string base64_encode(const std::vector<unsigned char>& data) {
    BIO* bio;
    BIO* b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    BIO_push(b64, bio);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines

    BIO_write(b64, data.data(), data.size());
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bufferPtr);
    BIO_set_close(b64, BIO_NOCLOSE);
    BIO_free_all(b64);

    return std::string(bufferPtr->data, bufferPtr->length);
}


// Function to decode Base64 to binary data
std::vector<unsigned char> base64_decode(const std::string& encoded) {
    BIO* bio;
    BIO* b64;
    std::vector<unsigned char> decoded_data(encoded.length() * 3 / 4); // Allocate enough space

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(encoded.data(), encoded.length());
    BIO_push(b64, bio);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines

    int decoded_length = BIO_read(b64, decoded_data.data(), encoded.length());
    decoded_data.resize(decoded_length); // Resize to actual decoded length
    BIO_free_all(b64);

    return decoded_data;
}

bool isIPLockedOut(const std::string& ipAddress) {
   sqlite3* db;
   sqlite3_stmt* stmt;
     int64_t lockoutEndTime;
     int attempts;
  //  Logger& logger = Logger::getInstance();
    // logger.log(Logger::INFO,"\n============isIPLockedOut ============\n");
    // Open the database
    int rc = sqlite3_open(DB_PATH_UM, &db);
    if (rc != SQLITE_OK) {
        // logger.log(Logger::INFO,"\n============can't open database ============\n");
        return false;
    }

 
    const char* selectSQL = "SELECT attempts, lockoutEndTime FROM IPInfo WHERE ipAddress = ?";
    // logger.log(Logger::INFO,"\n============isIPLockedOut  database opened successfully============\n");
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, ipAddress.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        attempts = sqlite3_column_int(stmt, 0);
        lockoutEndTime = sqlite3_column_int64(stmt, 1);
        
        // logger.log(Logger::INFO,"\n============isIPLockedOut lockoutEndTime\n",lockoutEndTime);
        // logger.log(Logger::INFO,"\n============isIPLockedOut attempts\n",attempts);
        // sqlite3_finalize(stmt);
        // sqlite3_close(db);
        int64_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
       
        // logger.log(Logger::INFO,"\n============isIPLockedOut now_time\n",now_time);
        sqlite3_finalize(stmt);
        if(attempts>=(MAX_ATTEMPTS)) 
        {
          if(lockoutEndTime >= now_time)
          {
            // logger.log(Logger::INFO,"\n============isIPLockedOut success\n");
            
            sqlite3_close(db);
            return true;
          }
          else{
             char sql_query[256] = {0};
            memset(sql_query, '\0', sizeof(sql_query));
            snprintf(sql_query, sizeof(sql_query), "DELETE FROM IPInfo WHERE ipAddress = '%s';", ipAddress.c_str());
             sqlite3_exec(db, sql_query, NULL, NULL, NULL);
            // if (rc != SQLITE_OK) {
                // logger.log(Logger::ERROR,"Failed to execute statement in sql step: %s\n", sqlite3_errmsg(db));
              sqlite3_close(db);
                
              return false;
            // }
				
          }
        }
        else
        {
          //  sqlite3_finalize(stmt);
          sqlite3_close(db);
          //  logger.log(Logger::INFO,"\n============isIPLockedOut  account not locked attempts============\n",attempts);
          return false;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    //  logger.log(Logger::INFO,"\n============isIPLockedOut  account not locked============\n");
    return false;
}

std::pair<bool, int> InsertUpdateIP(const std::string& ipAddress)
 {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int attempts =0;
    
    int64_t lockoutEndTime_sec;
    
    // Logger& logger = Logger::getInstance();
    // logger.log(Logger::INFO,"\n============InsertUpdateIP============\n");
    // Open the database
    int rc = sqlite3_open(DB_PATH_UM, &db);
    if (rc != SQLITE_OK) {
        // logger.log(Logger::ERROR,"\n============can't open database ============\n");
        return {false, attempts};
    }
    const char* selectSQL = "SELECT attempts, lockoutEndTime FROM IPInfo WHERE ipAddress = ?";
    // logger.log(Logger::INFO,"\n============InsertUpdateIP opened database successfully============\n");
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
         sqlite3_close(db);
        // logger.log(Logger::ERROR,"\n============InsertUpdateIP Error preparing select statement============\n");
        return {false, attempts};
    }

    sqlite3_bind_text(stmt, 1, ipAddress.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
         attempts = sqlite3_column_int(stmt, 0);
        //  lockoutEndTime = sqlite3_column_int64(stmt, 1);
        // logger.log(Logger::INFO,"\n============InsertUpdateIP attempts in database============\n",attempts);
         auto lockoutEndTime = std::chrono::system_clock::now() + std::chrono::seconds{LOCKOUT_ENDTIME};
        lockoutEndTime_sec = std::chrono::duration_cast<std::chrono::seconds>(lockoutEndTime.time_since_epoch()).count();
        if (attempts >= (MAX_ATTEMPTS)) {
            
            // logger.log(Logger::INFO,"\n============InsertUpdateIP attempts are greater than max============ lockoutEndTime_sec\n",lockoutEndTime_sec);
        
        } else {
          
            attempts= attempts+1;
        }

        sqlite3_finalize(stmt);
        // logger.log(Logger::INFO,"\n============InsertUpdateIP  attempts============\n",attempts);
        // logger.log(Logger::INFO,"\n============InsertUpdateIP ipaddress============\n",ipAddress.c_str());
       
        // logger.log(Logger::INFO,"\n============InsertUpdateIP lockoutEndTime_sec============\n",lockoutEndTime_sec);
        char sql_query[512] = {0};
        memset(sql_query, '\0', sizeof(sql_query));
        snprintf(sql_query, sizeof(sql_query), "UPDATE IPInfo SET attempts = %d, lockoutEndTime = %lld WHERE ipAddress = '%s';", attempts, lockoutEndTime_sec ,ipAddress.c_str());
        // if (1 == hon_system_manage_user_record(sql_query)) 
        // {
        //   return false;
        // }
        /* Execute SQL statement */
        rc = sqlite3_exec(db, sql_query, NULL, NULL, NULL);
        if (rc != SQLITE_OK) {
            // logger.log(Logger::ERROR,"Failed to execute statement in sql step: %s\n", sqlite3_errmsg(db));
          sqlite3_close(db);
          return {false, attempts};
          
        }
        sqlite3_close(db);
        // logger.log(Logger::ERROR,"\n============InsertUpdateIP success from update===========\n");
        return {true, attempts};

    } else {
        sqlite3_finalize(stmt);
        attempts = 1;
        const char* insertSQL = "INSERT INTO IPInfo (ipAddress, attempts, lockoutEndTime) VALUES (?, 1, ?)";
        rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
           sqlite3_close(db);
            // logger.log(Logger::ERROR,"\n============InsertUpdateIP Error preparing insert statement===========\n");
            // std::cerr << "Error preparing insert statement: " << sqlite3_errmsg(db) << std::endl;
            return {false, attempts};
        }
        lockoutEndTime_sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        // logger.log(Logger::INFO,"\n============InsertUpdateIP in insert========== lockoutEndTime_sec\n",lockoutEndTime_sec);
        sqlite3_bind_text(stmt, 1, ipAddress.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, lockoutEndTime_sec);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
          sqlite3_finalize(stmt);
          sqlite3_close(db);
          // logger.log(Logger::ERROR,"\n============InsertUpdateIP Error inserting record===========\n");
          return {false, attempts};
            // std::cerr << "Error inserting record: " << sqlite3_errmsg(db) << std::endl;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    // logger.log(Logger::ERROR,"\n============InsertUpdateIP success===========\n");
    return {true, attempts};
}
// Function to print bytes in hex format
std::string print_hex(const std::vector<unsigned char>& data) {
  std::ostringstream oss;
  for (unsigned char c : data) {
    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
  }
  std::string hex_data = oss.str();
  return hex_data;  
}

std::string aes_decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv,bool return_hex_flag) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    // int ciphertext_len = 320;
    std::vector<unsigned char> plaintext(ciphertext.size());
    // std::vector<unsigned char> plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH); // Ensure enough space for padding
    int len;
    int plaintext_len;
    // Logger& logger = Logger::getInstance();
    if(!ctx) {
        // logger.log(Logger::INFO, "\n============Decrypt  -- failed in EVP_CIPHER_CTX_new============\n");
        return "";
    }

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data())) {
        // logger.log(Logger::INFO, "\n============Decrypt  -- failed in EVP_DecryptInit_ex============\n");
        return "";
    }

    if(1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size())) {
        // logger.log(Logger::INFO, "\n============Decrypt  -- failed in EVP_DecryptUpdate============\n");
        return "";
    }
    plaintext_len = len;
    // Log lengths to check padding
    // logger.log(Logger::INFO, "Ciphertext length in decryption: " ,std::to_string(ciphertext.size()));
    // logger.log(Logger::INFO, "\n Plaintext length in decryption: " ,std::to_string(plaintext_len));

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
        // logger.log(Logger::INFO, "\n============Decrypt  -- failed in EVP_DecryptFinal_ex============\n");
        return "";
    }
    plaintext_len += len;

    // Log lengths to check padding
    // logger.log(Logger::INFO, "Ciphertext length in decryption: " ,std::to_string(ciphertext.size()));
    // logger.log(Logger::INFO, "\n Plaintext length in decryption: " ,std::to_string(plaintext_len));

    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);
    if(return_hex_flag == 1)
    {
      return print_hex(plaintext);
    }
    else{
      return std::string(plaintext.begin(), plaintext.end());
    }
    
}


std::vector<unsigned char> aes_encrypt(const std::string& plaintext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv) {

    std::vector<unsigned char> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0;
    int ciphertext_len = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    // Logger& logger = Logger::getInstance();
    if(!ctx)
    {
      // logger.log(Logger::INFO,"\n============Encrypt  -- failed in EVP_CIPHER_CTX_new============\n");
      return ciphertext;
    }
    

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()))
    {
      // logger.log(Logger::INFO,"\n============Encrypt  -- failed in EVP_EncryptInit_ex============\n");
      return ciphertext;
    }
    if(1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size()))
    {
      // logger.log(Logger::INFO,"\n============Encrypt  -- failed in EVP_EncryptUpdate============\n");
      return ciphertext;
    }
    ciphertext_len = len;
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len))
    {
      // logger.log(Logger::INFO,"\n============Encrypt  -- failed in EVP_EncryptFinal_ex============\n");
      return ciphertext;
    }
    ciphertext_len += len;
 // Log lengths to check padding
    // logger.log(Logger::INFO, "Plaintext length in encryption: " , std::to_string(plaintext.size()));
    // logger.log(Logger::INFO, "\nCiphertext length in encryption: " ,std::to_string(ciphertext_len));
    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    
    return ciphertext;
}

std::string sha256(const std::string& str) 
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

std::string generate_session_id(const std::string& ip, const std::string& username, const std::string& expiry_token) {
    std::string combined = ip + username + expiry_token;
    return sha256(combined);
}
std::string current_time_get() {
  char current_time[30];
  time_t rawtime;
  time(&rawtime);
  struct tm *time_info = localtime(&rawtime);
  strftime(current_time, 30, "%FT%T", time_info);
  std::string current_time_s = current_time;

  return current_time_s;
}

time_t standard_to_timestamp(std::string standard_time) {
  time_t stamp_time = 0;
  struct tm *timeinfo;

  // get current timeinfo and modify it to the user's choice
  time(&stamp_time);
  timeinfo = localtime(&stamp_time);
  timeinfo->tm_year = stoi(standard_time.substr(0, 4)) - 1900;
  timeinfo->tm_mon = stoi(standard_time.substr(5, 2)) - 1;
  timeinfo->tm_mday = stoi(standard_time.substr(8, 2));
  timeinfo->tm_hour = stoi(standard_time.substr(11, 2));
  timeinfo->tm_min = stoi(standard_time.substr(14, 2));
  timeinfo->tm_sec = stoi(standard_time.substr(17, 2));
  stamp_time = mktime(timeinfo);

  return stamp_time;
}

bool readSessionData(const std::string& ip_address, SessionData& session_data) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    // Logger& logger = Logger::getInstance();
    const char* select_sql ="SELECT session_id, ip_address, session_key, initialization_vector,username,expiry_time FROM session_history WHERE session_id = ?;";

    // Open the database
    int rc = sqlite3_open(DB_PATH_UM, &db);
    if (rc != SQLITE_OK) {
        // logger.log(Logger::INFO,"\n============can't open database ============\n");
        return false;
    }

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        // logger.log(Logger::INFO,"\n============error in sqlite3_prepare_v2 statement ============\n");
        sqlite3_close(db);
        return false;
    }
    // logger.log(Logger::INFO,"\n============readSessionData session_id is ============\n",session_data.session_id);
    // Bind the ip_address
    sqlite3_bind_text(stmt, 1, session_data.session_id.c_str(), -1, SQLITE_STATIC);

    // Execute the statement and read the results
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
//         // Read the session_id
//         ip_address = reinterpret_cast<const unsigned char*>(sqlite3_column_text(stmt, 1));
// logger.log(Logger::INFO,"\n============readSessionData ip_address is ============\n",ip_address);


        // Read the iv
        const unsigned char* iv_data = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(stmt, 3));
        int iv_size = sqlite3_column_bytes(stmt, 3);
       
        session_data.initialization_vector.assign(iv_data, iv_data + iv_size);
        //  std::ostringstream oss;
        //         for (unsigned char c :  session_data.initialization_vector) {
        //             oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        //         }
        //         std::string iv_data_hex = oss.str();
        // logger.log(Logger::INFO,"\n============readSessionData iv_size is ============\n",iv_size);
        // logger.log(Logger::INFO,"\n============readSessionData iv_data is ============\n",iv_data_hex);
        // Read the key
        const unsigned char* key_data = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(stmt, 2));
        int key_size = sqlite3_column_bytes(stmt, 2);
        // logger.log(Logger::INFO,"\n============readSessionData key_data is ============\n",key_data);
        session_data.session_key.assign(key_data, key_data + key_size);
        // std::ostringstream oss_test;
        //         for (unsigned char c :  session_data.session_key) {
        //             oss_test << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        //         }
        //         std::string key_data_hex = oss_test.str();
        // logger.log(Logger::INFO,"\n============readSessionData key_size is ============\n",key_size);
        // logger.log(Logger::INFO,"\n============readSessionData key_data_hex is ============\n",key_data_hex);
    } else {
        //  logger.log(Logger::INFO,"\n============readSessionData no data found for session_id ============\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

bool storeSessionHistory(const std::string& session_id, const std::string& remote_address, 
                         const std::vector<unsigned char>& initialization_vector, const std::vector<unsigned char>& session_key,const std::string& username, long int expiry_time) {
    sqlite3* db;
    char* err_msg = nullptr;

    // Open the database
    int rc = sqlite3_open(DB_PATH_UM, &db);
    if (rc != SQLITE_OK) {
      return false;
    }

    // Prepare the SQL statement
    const char* insert_sql = "INSERT INTO session_history (session_id, ip_address, session_key, initialization_vector,username,expiry_time) VALUES (?, ?, ?, ?,?,?);";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    // Bind the values
    sqlite3_bind_text(stmt, 1, session_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, remote_address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 3, session_key.data(), session_key.size(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 4, initialization_vector.data(), initialization_vector.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 6, expiry_time);
    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

std::vector<unsigned char> jwt_token_get(std::string session_id, std::string remote_address, int auth,long expiretime,std::string username) {
SessionData sessionid(session_id);
// Logger& logger = Logger::getInstance();
  std::string token =
      jwt::create()
          .set_issuer("auth0")
          .set_type("JWS")
          .set_payload_claim("remote_address", jwt::claim(remote_address))
          .set_payload_claim("auth", jwt::claim(std::to_string(auth)))
          .set_payload_claim("sessionId", jwt::claim(session_id))
          .set_issued_at(std::chrono::system_clock::now())
          // .set_expires_at(std::chrono::system_clock::now() +
          //                 std::chrono::seconds{expiretime})
          .set_expires_at(std::chrono::system_clock::time_point(std::chrono::seconds(expiretime)))
          .sign(jwt::algorithm::hs256{SECRET});
          // logger.log(Logger::INFO,"\n============jwt_token_get token is ============\n",token);
          // Encrypt the JWT
    // std::vector<unsigned char> key(32); // AES-256 key size is 32 bytes
    // std::vector<unsigned char> iv(16);  // AES block size is 16 bytes

    // // Generate random key and IV
    // RAND_bytes(key.data(), key.size());
    // RAND_bytes(iv.data(), iv.size());
                
   
    std::vector<unsigned char> encrypted_token = aes_encrypt(token,sessionid.session_key, sessionid.initialization_vector);
    //  std::ostringstream oss;
    //             for (unsigned char c : encrypted_token) {
    //                 oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    //             }
    //             std::string encrypted_token_hex = oss.str();

                
    // logger.log(Logger::INFO,"\n============encrypted token in jwt_token_get============\n",encrypted_token_hex);

     //  char sql_query[256] = {0};
  //   memset(sql_query, '\0', 1);
  //   snprintf(sql_query, sizeof(sql_query), "INSERT INTO session_history (session_id, ip_address,key,iv) VALUES ('%s','%s','%s','%s');", session_id.c_str(),remote_address.c_str(),session_data.key, session_data.iv);
  //   hon_system_manage_user_record(sql_query);
  // Store the session history in the database
if(storeSessionHistory(session_id.c_str(),remote_address.c_str(), sessionid.initialization_vector, sessionid.session_key,username.c_str(),expiretime) == false)
{
  return std::vector<unsigned char>();
}
//  SessionData session_data("");
//  readSessionData(remote_address.c_str(),session_data);
//   std::string decrypted_token = decrypt(encrypted_token, session_data.session_key, session_data.initialization_vector);
//   logger.log(Logger::INFO,"\n============decrypted token in jwt_token_get============\n",decrypted_token);

  return encrypted_token;
}

int jwt_token_verify(std::vector<unsigned char> encrypted_token, std::string session_id, HttpRequest &Req) {

  // Logger& logger = Logger::getInstance();

  SessionData session_data(session_id);
  //  logger.log(Logger::INFO,"\n============jwt_token_verify  session_id ============\n",session_data.session_id);
   char sql_query[256] = {0};
    memset(sql_query, '\0', 1);
    snprintf(sql_query, sizeof(sql_query), "SELECT * FROM session_history WHERE session_id = '%s' LIMIT 1;", session_id.c_str());
    if (0 == hon_system_record_exists(sql_query)) {
      // logger.log(Logger::INFO,"\n============jwt_token_verify failed to hon_system_record_exists ============\n");
        
      return API_FORBIDDEN;
    }
    if(readSessionData(Req.RemoteAddress,session_data) == false)
    { 
      return API_FORBIDDEN;
      // logger.log(Logger::INFO,"\n============jwt_token_verify failed to read the session data ============\n");
    }
    
  // logger.log(Logger::INFO,"\n============jwt_token_verify encrypted_token length is ============\n",std::to_string(encrypted_token.size()));
//  std::ostringstream oss_key;
//                 for (unsigned char c :  session_data.session_key) {
//                     oss_key << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
//                 }
//                 std::string key_data_hex = oss_key.str();
//         logger.log(Logger::INFO,"\n============jwt_token_verify key_data_hex is ============\n",key_data_hex);
// std::ostringstream oss_iv;
//                 for (unsigned char c :  session_data.initialization_vector) {
//                     oss_iv << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
//                 }
//                 std::string iv_data_hex = oss_iv.str();
//         logger.log(Logger::INFO,"\n============jwt_token_verify iv_data_hex is ============\n",iv_data_hex);

  std::string decrypted_token = aes_decrypt(encrypted_token, session_data.session_key, session_data.initialization_vector,0);

  // logger.log(Logger::INFO,"\n============jwt_token_verify decrypted_token is ============\n",decrypted_token);
  auto decoded = jwt::decode(decrypted_token);
  std::string payload = decoded.get_payload();

  nlohmann::json payload_json = nlohmann::json::parse(payload);
  int expire_time = payload_json.at("exp");
  // logger.log(Logger::INFO,"\n============jwt_token_verify exp is ============\n",expire_time);
  int now_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::string auth = payload_json.at("auth");
  std::string remote_address = payload_json.at("remote_address");
  //  logger.log(Logger::INFO,"\n============jwt_token_verify remote_address is ============\n",remote_address);
  if(remote_address != Req.RemoteAddress)
  {
    return API_FORBIDDEN;
  }
  if (expire_time <= now_time) {
    memset(sql_query, '\0', 1);
    snprintf(sql_query, sizeof(sql_query), "DELETE FROM session_history WHERE session_id = '%s';", session_id.c_str());
    if (0 == hon_system_manage_user_record(sql_query)) {
      return API_FORBIDDEN;
    }
    else
    {
      return API_UNAUTHORIZED;
    }
   
  }
  else
  {
    auto verifier = jwt::verify()
                      .allow_algorithm(jwt::algorithm::hs256{SECRET})
                      .with_issuer("auth0");
    verifier.verify(decoded);
    Req.UserLevel = stoi(auth);
    Req.SessionId = payload_json.at("sessionId");
    if(Req.UserLevel == ADMIN)
    {
      return ADMIN;
    }
    else if(Req.UserLevel == OPERATOR)
    {
      return OPERATOR;
    }
    else if(Req.UserLevel == USER)
    {
      return USER;
    }
  }
  

}
unsigned char FromHex(unsigned char x) {
  unsigned char y;
  if (x >= 'A' && x <= 'Z')
    y = x - 'A' + 10;
  else if (x >= 'a' && x <= 'z')
    y = x - 'a' + 10;
  else if (x >= '0' && x <= '9')
    y = x - '0';
  else
    assert(0);
  return y;
}

std::string DecodeURI(std::string &str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    if (str[i] == '+')
      strTemp += ' ';
    else if (str[i] == '%') {
      assert(i + 2 < length);
      unsigned char high = FromHex((unsigned char)str[++i]);
      unsigned char low = FromHex((unsigned char)str[++i]);
      strTemp += high * 16 + low;
    } else
      strTemp += str[i];
  }
  return strTemp;
}


void convert_space_in_param(std::string &param) {
  std::string space_string = "%20";
  size_t space_index = param.find(space_string);
  while (space_index!=std::string::npos)
  {
    param.replace(space_index,3," ");
    space_index = param.find(space_string);
  }
}


void getStringFromErrorCode(RetCode ret_code, std::string &error_string) {
  switch (ret_code) {
	case RetCode::API_SUCCESS:
	  error_string = "API Successful";
	  break;
	case RetCode::API_FAILURE:
	  error_string = "Unknown request failure. Please try again";
	  break;
	case RetCode::API_INAVLID_ARGUMENT:
	  error_string = "Invalid argument received in request. Please check the request and try again.";
	  break;
	case RetCode::API_SQL_DB_FAILURE:
	  error_string = "Request failed from server database. Please check the request and try again.";
	  break;
	case RetCode::API_MEM_ALLOC_FAILURE:
	  error_string = "Unknown Server error. Please try again in some time.";
	  break;
	case RetCode::API_MAX_LIMIT:
	  error_string = "Maximum limit reached. Please try again";
	  break;
	case RetCode::API_REQUEST_CHECK_SUCCESS:
	  error_string = "API Request check success";
	  break;
	case RetCode::API_REQUEST_CHECK_FAILURE:
	  error_string = "Invalid API Request. Please check the request and try again.";
	  break;
	case RetCode::API_NAME_LENGTH_FAILURE:
	  error_string = "Name length failure";
	  break;
	case RetCode::API_COORDINATE_INVALID:
	  error_string = "Invalid coordinates passed. Please check the coordinates and try again.";
	  break;
	case RetCode::RULE_NAME_LENGTH_FAILURE:
	  error_string = "Rule name length should be between 8 and 24 characters";
	  break;
	case RetCode::RULE_COUNT_EXCEEDS:
	  error_string = "Maximum rule count exceeded. Please delete a rule and try again.";
	  break;
	case RetCode::API_NOT_IMPLEMENTED:
	  error_string = "API not implemented";
	  break;
	case RetCode::RULE_NAME_EXISTS:
	  error_string = "Rule name already exists. Please choose a different name.";
	  break;
  case RetCode::INVALID_RULE_NAME:
    error_string = "Invalid rule name. Rule name cannot start with number or have special characters.";
    break;
  case RetCode::NON_CONVEX_COORDINATES:
    error_string = "Trespassing Zone is self intersecting. Please create a non-intersecting zone.";
    break;
  case RetCode::TRESPASS_COORDINATE_SIZE_FAILURE:
    error_string = "Trespassing zone coordinates should be more than 3 and upto 8.";
    break;
  case RetCode::TRIPWIRE_COORDINATE_SIZE_FAILURE:
    error_string = "Tripwire accepts only 2 coordinates.";
    break;
	default:
	  error_string = "Unknown Error. Please try again";
	  break;
  }
}




#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

std::string ipv4_address_get() {
  char ip[32] = {NULL};
  int ret;
  int sock;
  struct ifreq ifr;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return NULL;
  }
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, "eth0", sizeof("eth0") - 1);
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';
  ret = ioctl(sock, SIOCGIFADDR, &ifr);
  if (ret) {
    strncpy(ifr.ifr_name, "usb0", sizeof("usb0") - 1);
    ret = ioctl(sock, SIOCGIFADDR, &ifr);
    if (ret) {
      strncpy(ifr.ifr_name, "wlan0", sizeof("wlan0") - 1);
      ret = ioctl(sock, SIOCGIFADDR, &ifr);
    }
  }
  sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr));

  if (sock > 0) {
    close(sock);
  }

  return std::string(ip);
}



} // namespace cgi
} // namespace rockchip
