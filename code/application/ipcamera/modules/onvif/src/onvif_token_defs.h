/**
 * @file onvif_token_defs.h
 * @brief Centralized ONVIF Configuration Token Definitions
 *
 * ALL ONVIF configuration tokens MUST be generated using the macros and
 * helper functions defined in this file.  This guarantees that every layer
 * of the ONVIF stack (media service, profiles layer, media2, device, etc.)
 * produces and parses the exact same token strings.
 *
 * Token naming convention (ONVIF-compliant):
 *   VideoSource_0           – physical camera sensor
 *   VideoSourceConfig_0     – video source configuration (one per sensor)
 *   Profile_1 … Profile_3   – media profiles  (1-indexed, matching ONVIF spec)
 *   VideoEncoderToken_1 … 3 – video encoder configurations
 *   AudioSource_1           – audio input
 *   AudioEncoderToken_1     – audio encoder configuration
 *   VideoAnalyticsToken_1   – analytics configuration
 *   MetaDataConfigToken_1   – metadata configuration
 */

#ifndef ONVIF_TOKEN_DEFS_H
#define ONVIF_TOKEN_DEFS_H

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Token format strings  (printf-style)                               */
/* ------------------------------------------------------------------ */

/** Video source – one per physical sensor, 0-indexed */
#define ONVIF_TOKEN_FMT_VIDEO_SOURCE          "VideoSource_%d"

/** Video source configuration – one per sensor, 0-indexed */
#define ONVIF_TOKEN_FMT_VIDEO_SOURCE_CONFIG   "VideoSourceConfig_%d"

/** Media profile – 1-indexed to match ONVIF convention */
#define ONVIF_TOKEN_FMT_PROFILE               "Profile_%d"

/** Video encoder – 1-indexed, one per media profile */
#define ONVIF_TOKEN_FMT_VIDEO_ENCODER         "VideoEncoderToken_%d"

/** Video encoder name (human readable) */
#define ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME    "VideoEncoder_%d"

/** Audio source – 1-indexed */
#define ONVIF_TOKEN_FMT_AUDIO_SOURCE          "AudioSource_%d"

/** Audio encoder – 1-indexed */
#define ONVIF_TOKEN_FMT_AUDIO_ENCODER         "AudioEncoderToken_%d"

/** Video analytics – 1-indexed, one per media profile */
#define ONVIF_TOKEN_FMT_VIDEO_ANALYTICS       "VideoAnalyticsToken_%d"

/** Video analytics name */
#define ONVIF_TOKEN_FMT_VIDEO_ANALYTICS_NAME  "VideoAnalyticsConfig_%d"

/** Metadata configuration – 1-indexed */
#define ONVIF_TOKEN_FMT_METADATA              "MetaDataConfigToken_%d"

/** Metadata configuration name */
#define ONVIF_TOKEN_FMT_METADATA_NAME         "MetaDataConfig_%d"

/* ------------------------------------------------------------------ */
/*  Profile names  (human readable, fixed)                             */
/* ------------------------------------------------------------------ */
#define ONVIF_PROFILE_NAME_MAIN   "Main_Profile"
#define ONVIF_PROFILE_NAME_SUB    "Sub_Profile"
#define ONVIF_PROFILE_NAME_THIRD  "Third_Profile"

/** Video source configuration name (single sensor) */
#define ONVIF_VIDEO_SOURCE_CONFIG_NAME  "VIDEO_SOURCE_CONFG"

/* ------------------------------------------------------------------ */
/*  Convenience macros – generate a token into a buffer                */
/* ------------------------------------------------------------------ */

/**
 * @brief Generate a token string into `buf` (max `sz` chars).
 * @param buf   destination buffer (char*)
 * @param sz    size of buffer
 * @param fmt   one of the ONVIF_TOKEN_FMT_* macros
 * @param idx   numeric index
 */
#define ONVIF_TOKEN_GEN(buf, sz, fmt, idx) \
    snprintf((buf), (sz), (fmt), (idx))

/* ------------------------------------------------------------------ */
/*  Convenience inline helpers – parse an index back from a token      */
/* ------------------------------------------------------------------ */

/**
 * @brief Parse the integer index from a token string.
 * @param token   the token string to parse
 * @param fmt     the format that was used to generate it
 * @param out_idx pointer to int receiving the parsed index
 * @return 0 on success, -1 on parse failure
 */
static inline int onvif_token_parse_index(const char *token,
                                           const char *fmt,
                                           int *out_idx)
{
    if (!token || !fmt || !out_idx) return -1;
    if (sscanf(token, fmt, out_idx) != 1) return -1;
    return 0;
}

/**
 * @brief Convert a Profile token ("Profile_1") to a 0-based channel index.
 * @return channel index (0, 1, 2, …) or -1 on failure
 */
static inline int onvif_profile_token_to_channel(const char *token)
{
    int profile_num = -1;
    if (onvif_token_parse_index(token, ONVIF_TOKEN_FMT_PROFILE,
                                 &profile_num) != 0)
        return -1;
    return profile_num - 1;   /* Profile_1 → channel 0 */
}

/**
 * @brief Convert a 0-based channel index to a 1-based profile number.
 */
static inline int onvif_channel_to_profile_num(int channel)
{
    return channel + 1;
}

/**
 * @brief Convert a VideoEncoderToken ("VideoEncoderToken_1") to a 0-based channel index.
 * @return channel index (0, 1, 2, …) or -1 on failure
 */
static inline int onvif_encoder_token_to_channel(const char *token)
{
    int enc_num = -1;
    if (onvif_token_parse_index(token, ONVIF_TOKEN_FMT_VIDEO_ENCODER,
                                 &enc_num) != 0)
        return -1;
    return enc_num - 1;   /* VideoEncoderToken_1 → channel 0 */
}

#endif /* ONVIF_TOKEN_DEFS_H */
