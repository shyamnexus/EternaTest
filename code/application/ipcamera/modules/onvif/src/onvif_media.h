#ifndef ONVIF_MEDIA_H
#define ONVIF_MEDIA_H

#define TOKEN_LENGTH 64

// Platform selection should be set by build system, not unconditionally here
// #define LUCKFOX

// FIX: Declare as extern to avoid multiple-definition errors when header is
// included from multiple translation units. Definition moves to onvif_media.c.
extern char *timeout;

typedef enum
{
    TRANSPORT_PROTOCOL_UDP = 0,
    TRANSPORT_PROTOCOL_TCP,
    TRANSPORT_PROTOCOL_RTSP,
    TRANSPORT_PROTOCOL_HTTP,
    TRANSPORT_PROTOCOL_HTTPS,
    MAX_TRANSPORT_PROTOCOL
}TRANSPORT_PROTOCOL;
//@todo, later need to store in .ini file, so at power on the values will be same
/*
typedef struct
{
    char confToken[TOKEN_LENGTH];
    char srcToken[TOKEN_LENGTH];
    char name[TOKEN_LENGTH]; // Name Token
    int useCount;
    int maxNumOfProfiles;
    int boundX;
    int minX;
    int maxX;
    int boundY;
    int minY;
    int maxY;
    int boundWidth;
    int minboundWidth;
    int maxboundWidth;
    int boundHeight;
    int minboundHeight;
    int maxboundHeight;
}VIDEO_SRC_CONFIG_PARAM;
*/
typedef struct
{
    char m_ca_profileToken[TOKEN_LENGTH];
    char m_ca_profileName[TOKEN_LENGTH];
}T_CREATE_CONFIG_INFO;

#define ONVIF_ENV_SENDER            1

typedef struct
{
    char m_ca_tokenName[TOKEN_LENGTH];
  //  unsigned short int m_ui16_commandID;
}T_ONVIF_CONFIG_INFO;
// FIX: Declare as extern to avoid multiple-definition errors.
// Definitions move to onvif_media.c.
extern bool testProfilecreated;
extern bool testProfileAddSourceConfig;
extern bool testProfileAddVideoEncoderConfig;
extern bool testProfileAddMetaDataConfig;
//bool testProfileRemoveVideoEncoderConfig = false;

#endif /* ONVIF_MEDIA_H */
