#ifndef GLOBAL_H
#define GLOBAL_H

#define GLOBAL_FEATURES 0x7
#define GLOBAL_MODEL    "AppleTV2,1"
#define GLOBAL_VERSION  "220.68"

/* use old protocol  for AES key if User-Agent string is contained in these strings */
/* replace xxx by any new User-Agent string as needed */
#define OLD_PROTOCOL_AUDIO_CLIENT_LIST "AirMyPC/2.0;AirPlay/260.26;AirPlay/320.20;xxx"
#define OLD_PROTOCOL_VIDEO_CLIENT_LIST "AirMyPC/2.0;xxx"

#define MAX_HWADDR_LEN 6

#endif
