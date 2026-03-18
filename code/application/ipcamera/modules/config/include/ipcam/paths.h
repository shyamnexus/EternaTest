#pragma once

/**
 * @file paths.h
 * @brief Centralized path definitions for the IP camera application
 * 
 * All system paths should be defined here to avoid magic strings throughout
 * the codebase. This makes it easy to change paths for different deployments
 * and ensures consistency.
 */

namespace ipcam {
namespace paths {

// =============================================================================
// Configuration Paths
// =============================================================================
constexpr const char* kConfigDir = "/etc/ipcamera/configs";
constexpr const char* kConfigDirActive = "/etc/ipcamera/configs/config.d";
constexpr const char* kConfigDirFactory = "/etc/ipcamera/configs/config.factory.d";

// =============================================================================
// Database Paths
// =============================================================================
constexpr const char* kDatabaseDir = "/mnt/app/ipcamera/db";
constexpr const char* kUserDatabase = "/mnt/app/ipcamera/db/usermng.db";
constexpr const char* kSecureDatabase = "/mnt/app/ipcamera/db/secure.db";
constexpr const char* kUsersDatabase = "/mnt/app/ipcamera/db/users.db";
constexpr const char* kUsersDbLegacy = "/etc/ipcamera/users.db";

// =============================================================================
// Network Configuration Paths
// =============================================================================
constexpr const char* kNetworkInterfacesDir = "/etc/network/interfaces.d";
constexpr const char* kEth0Config = "/etc/network/interfaces.d/eth0.conf";

// =============================================================================
// SSL/TLS Certificate Paths
// =============================================================================
constexpr const char* kSslDir = "/etc/nginx/ssl";
constexpr const char* kSslCertificate = "/etc/nginx/ssl/Server.crt";
constexpr const char* kSslPrivateKey = "/etc/nginx/ssl/Server.key";

// =============================================================================
// Cache and Runtime Paths
// =============================================================================
constexpr const char* kCacheDir = "/var/lib/ipcamera";
constexpr const char* kLogDir = "/var/log";
constexpr const char* kLogFile = "/var/log/ipcamd.log";
constexpr const char* kTmpDir = "/tmp";

// =============================================================================
// Storage Paths
// =============================================================================
constexpr const char* kSdCardMount = "/mnt/sd";
constexpr const char* kRecordingDir = "/mnt/sd/recordings";
constexpr const char* kSnapshotDir = "/mnt/sd/snapshots";

// =============================================================================
// ONVIF Paths
// =============================================================================
constexpr const char* kOnvifPasswordsFile = "/etc/ipcamera/onvif_passwords.conf";

// =============================================================================
// AI/Analytics Paths
// =============================================================================
constexpr const char* kFaceDatabase = "/mnt/app/face_db";
constexpr const char* kModelDir = "/usr/share/ipcamera/models";

} // namespace paths
} // namespace ipcam
