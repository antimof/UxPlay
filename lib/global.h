#ifndef GLOBAL_H
#define GLOBAL_H

#define GLOBAL_FEATURES 0x7
#define GLOBAL_MODEL    "AppleTV2,1"
#define GLOBAL_VERSION  "220.68"

/* use old protocol (unhashed AESkey for audio decryption if clients source version is not greater than 330.x.x */
#define OLD_PROTOCOL_AUDIO_CLIENT_SOURCEVERSION "330.0.0"

/* use old protocol  for audio or video AES key if client's User-Agent string is contained in these strings */
/* replace xxx by any new User-Agent string as needed */
#define OLD_PROTOCOL_AUDIO_CLIENT_USER_AGENT_LIST "AirMyPC/2.0;xxx"
#define OLD_PROTOCOL_VIDEO_CLIENT_USER_AGENT_LIST "AirMyPC/2.0;xxx"

#define DECRYPTION_TEST 2

#define MAX_HWADDR_LEN 6

#endif
