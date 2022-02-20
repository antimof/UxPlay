#ifndef GLOBAL_H
#define GLOBAL_H

#define GLOBAL_MODEL    "AppleTV3,2"
#define GLOBAL_VERSION  "220.68"

/* use old protocol for audio AES key if client's User-Agent string is contained in these strings */
/* replace xxx by any new User-Agent string as needed */
#define OLD_PROTOCOL_CLIENT_USER_AGENT_LIST "AirMyPC/2.0;xxx"

#define DECRYPTION_TEST 0    /* set to 1 or 2 to examine audio decryption */

#define MAX_HWADDR_LEN 6

#endif
