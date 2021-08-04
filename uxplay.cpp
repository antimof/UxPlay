/**
 * RPiPlay - An open-source AirPlay mirroring server for Raspberry Pi
 * Copyright (C) 2019 Florian Draschbacher
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <stddef.h>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>

#include "log.h"
#include "lib/raop.h"
#include "lib/stream.h"
#include "lib/logger.h"
#include "lib/dnssd.h"
#include "renderers/video_renderer.h"
#include "renderers/audio_renderer.h"

#define VERSION "1.3"

#define DEFAULT_NAME "UxPlay"
#define DEFAULT_DEBUG_LOG false
#define DEFAULT_DISPLAY_WIDTH 1920
#define DEFAULT_DISPLAY_HEIGHT 1080
#define LOWEST_ALLOWED_PORT 1024
#define HIGHEST_PORT 65535


int start_server (std::vector<char> hw_addr, std::string name, unsigned short display_size[2],
                 unsigned short tcp[2], unsigned short udp[3], videoflip_t videoflip,
                 bool use_audio,  bool debug_log);

int stop_server ();

static bool running = false;
static dnssd_t *dnssd = NULL;
static raop_t *raop = NULL;
static video_renderer_t *video_renderer = NULL;
static audio_renderer_t *audio_renderer = NULL;

static void signal_handler (int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            running = 0;
            break;
    }
}

static void init_signals (void) {
    struct sigaction sigact;

    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
}

static int parse_hw_addr (std::string str, std::vector<char> &hw_addr) {
    for (int i = 0; i < str.length(); i += 3) {
        hw_addr.push_back((char) stol(str.substr(i), NULL, 16));
    }
    return 0;
}

std::string find_mac () {
    std::ifstream iface_stream("/sys/class/net/eth0/address");
    if (!iface_stream) {
        iface_stream.open("/sys/class/net/wlan0/address");
    }
    if (!iface_stream) return "";

    std::string mac_address;
    iface_stream >> mac_address;
    iface_stream.close();
    return mac_address;
}

#define MULTICAST 0
#define LOCAL 1
#define OCTETS 6
std::string random_mac () {
    char str[3];
    int octet = rand() % 64;
    octet = (octet << 1) + LOCAL;
    octet = (octet << 1) + MULTICAST;
    snprintf(str,3,"%02x",octet);
    std::string mac_address(str);
    for (int i = 1; i < OCTETS; i++) {
        mac_address = mac_address + ":";
        octet =  rand() % 256;
        snprintf(str,3,"%02x",octet);
        mac_address = mac_address + str;
    }
    return mac_address;
}

void print_info (char *name) {
    printf("UxPlay %s: An open-source AirPlay mirroring server based on RPiPlay\n", VERSION);
    printf("Usage: %s [-n name] [-s wxh] [-p [n]]\n", name);
    printf("Options:\n");
    printf("-n name   Specify the network name of the AirPlay server\n");
    printf("-s wxh    Set display resolution: width w height h default 1920x1080\n");
    printf("-f {H|V|R|L|I} horizontal/vertical flip; rotate 90R/90L/180 deg\n");
    printf("-p n      Use fixed UDP+TCP network ports n:n+1:n+2. (n>1023)\n");
    printf("-p        Use legacy UDP 6000:6001:7011 TCP 7000:7001:7100\n");
    printf("-r        use random MAC address (use for concurrent UxPlay's)\n");
    printf("-a        Turn audio off. video output only\n");
    printf("-d        Enable debug logging\n");
    printf("-v/-h     Displays this help and version information\n");
}

bool get_display_size (char *str, unsigned short *w, unsigned short *h) {
    // assume str  = wxh is valid if w and h are positive decimal integers with less than 5 digits.
    char *str1 = strchr(str,'x');
    if (str1 == NULL) return false;
    str1[0] = '\0'; str1++;
    if (str1[0] == '-') return false;  // first character of str is never '-'
    if (strlen(str) > 5 || strlen(str1) > 5 || !strlen(str) || !strlen(str1)) return false;
    char *end;
    *w = (unsigned short) strtoul(str, &end, 10);
    if (*end || *w == 0)  return false;
    *h = (unsigned short) strtoul(str1, &end, 10);
    if (*end || *h == 0) return false;
    return true;
}

bool get_lowest_port (char *str, unsigned short *n) {
    if (strlen(str) > 5) return false;
    char *end;
    *n = (unsigned short) strtoul(str, &end, 10);  // first character of str is never '-'
    if (*end) return false;
    if (*n < LOWEST_ALLOWED_PORT || *n > HIGHEST_PORT - 2 ) return false;
    return true;
}

bool get_videoflip (char *str, videoflip_t *videoflip) {
    if (strlen(str) > 1) return false;
    switch (str[0]) {
    case 'L':
        *videoflip = LEFT;
        break;
    case 'R':
        *videoflip = RIGHT;
        break;
    case 'I':
        *videoflip = INVERT;
        break;
    case 'H':
        *videoflip = HFLIP;
        break;
    case 'V':
        *videoflip = VFLIP;
        break;
    default:
        return false;
    }
    return true;
}

int main (int argc, char *argv[]) {
    init_signals();

    std::string server_name = DEFAULT_NAME;
    std::vector<char> server_hw_addr;
    bool use_audio = true;
    bool use_random_hw_addr = false;
    bool debug_log = DEFAULT_DEBUG_LOG;
    unsigned short display_size[2] = { (unsigned short) DEFAULT_DISPLAY_WIDTH,
                                       (unsigned short) DEFAULT_DISPLAY_HEIGHT };
    unsigned short tcp[2] = {0}, udp[3] = {0};
    videoflip_t  videoflip = NONE;
    
#ifdef AVAHI_COMPAT_NOWARN
    //suppress avahi_compat nag message
    char avahi_compat_nowarn[] = "AVAHI_COMPAT_NOWARN==1";
    putenv(avahi_compat_nowarn);
#endif
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "-n") {
            if (i == argc - 1) continue;
            server_name = std::string(argv[++i]);
        } else if (arg == "-s") {
            if (i == argc - 1 || argv[i + 1][0] == '-') {
                 fprintf(stderr,"invalid \"-s\" had no argument\n");
                 exit(1);
            }
            std::string value(argv[++i]);
            if (!get_display_size(argv[i], &display_size[0], &display_size[1])) {
                fprintf(stderr, "invalid \"-s %s\"; default is  \"-s 1920x1080\" (< 5 digits)\n",
                        value.c_str());
                exit(1);
            }
        } else if (arg == "-f") {
            if (i == argc - 1 || argv[i + 1][0] == '-') {
                fprintf(stderr,"invalid \"f\" had no argument\n");
                exit(1);
            }
            if (!get_videoflip(argv[++i], &videoflip)) {
                fprintf(stderr,"invalid \"-f %s\" , unknown videoflip type\n",argv[i]);
                exit(1);
            }
        } else if (arg == "-p") {
            if (i == argc - 1 || argv[i + 1][0] == '-') {
	        tcp[0] = 7100; tcp[1] = 7000;
	        udp[0] = 7011; udp[1] = 6001; udp[2] = 6000;
		continue;
            }
	    unsigned short n;
	    if(get_lowest_port(argv[++i], &n)) {
                for (int i = 0; i < 3; i++) {
		    udp[i] = n++;
                    if (i < 2) tcp[i] = udp[i];
                }
            } else {
                fprintf(stderr, "Error: \"-p %s\" is invalid (%d-%d is allowed)\n", 
                        argv[i], LOWEST_ALLOWED_PORT, HIGHEST_PORT);
                exit(1);
            }
        } else if (arg == "-r") {
            use_random_hw_addr  = true;
        } else if (arg == "-a") {
            use_audio = false;
        } else if (arg == "-d") {
            debug_log = !debug_log;
        } else if (arg == "-h" || arg == "-v") {
            print_info(argv[0]);
            exit(0);
        } else {
            LOGI("unknown option %s, skipping\n",argv[i]);
        }
    }

    if (udp[0]) LOGI("using network ports UDP %d %d %d TCP %d %d %d\n",
		     udp[0],udp[1], udp[2], tcp[0], tcp[1], tcp[1] + 1);

    std::string mac_address;
    if (!use_random_hw_addr) mac_address = find_mac();
    if (mac_address.empty()) {
        srand(time(NULL) * getpid());
        mac_address = random_mac();
        LOGI("using randomly-generated MAC address %s\n",mac_address.c_str());
    }
    parse_hw_addr(mac_address, server_hw_addr);
    mac_address.clear();

    if (start_server(server_hw_addr, server_name, display_size, tcp, udp,
                     videoflip,use_audio, debug_log) != 0) {
        return 1;
    }
    running = true;
    while (running) {
        sleep(1);
    }

    LOGI("Stopping...");
    stop_server();
}

// Server callbacks
extern "C" void conn_init (void *cls) {
    video_renderer_update_background(video_renderer, 1);
}

extern "C" void conn_destroy (void *cls) {
    video_renderer_update_background(video_renderer, -1);
}

extern "C" void audio_process (void *cls, raop_ntp_t *ntp, aac_decode_struct *data) {
    if (audio_renderer != NULL) {
        audio_renderer_render_buffer(audio_renderer, ntp, data->data, data->data_len, data->pts);
    }
}

extern "C" void video_process (void *cls, raop_ntp_t *ntp, h264_decode_struct *data) {
    video_renderer_render_buffer(video_renderer, ntp, data->data, data->data_len, data->pts, data->frame_type);
}

extern "C" void audio_flush (void *cls) {
    audio_renderer_flush(audio_renderer);
}

extern "C" void video_flush (void *cls) {
    video_renderer_flush(video_renderer);
}

extern "C" void audio_set_volume (void *cls, float volume) {
    if (audio_renderer != NULL) {
        audio_renderer_set_volume(audio_renderer, volume);
    }
}

extern "C" void log_callback (void *cls, int level, const char *msg) {
    switch (level) {
        case LOGGER_DEBUG: {
            LOGD("%s", msg);
            break;
        }
        case LOGGER_WARNING: {
            LOGW("%s", msg);
            break;
        }
        case LOGGER_INFO: {
            LOGI("%s", msg);
            break;
        }
        case LOGGER_ERR: {
            LOGE("%s", msg);
            break;
        }
        default:
            break;
    }

}

int start_server (std::vector<char> hw_addr, std::string name, unsigned short display_size[2],
                 unsigned short tcp[2], unsigned short udp[3], videoflip_t videoflip,
                 bool use_audio, bool debug_log) {
    raop_callbacks_t raop_cbs;
    memset(&raop_cbs, 0, sizeof(raop_cbs));
    raop_cbs.conn_init = conn_init;
    raop_cbs.conn_destroy = conn_destroy;
    raop_cbs.audio_process = audio_process;
    raop_cbs.video_process = video_process;
    raop_cbs.audio_flush = audio_flush;
    raop_cbs.video_flush = video_flush;
    raop_cbs.audio_set_volume = audio_set_volume;

    raop = raop_init(10, &raop_cbs);
    if (raop == NULL) {
        LOGE("Error initializing raop!");
        return -1;
    }

    raop_set_log_callback(raop, log_callback, NULL);
    raop_set_log_level(raop, debug_log ? RAOP_LOG_DEBUG : LOGGER_INFO);

    logger_t *render_logger = logger_init();
    logger_set_callback(render_logger, log_callback, NULL);
    logger_set_level(render_logger, debug_log ? LOGGER_DEBUG : LOGGER_INFO);

    if ((video_renderer = video_renderer_init(render_logger, name.c_str(), videoflip)) == NULL) {
        LOGE("Could not init video renderer");
        return -1;
    }

    if (! use_audio) {
        LOGI("Audio disabled");
    } else if ((audio_renderer = audio_renderer_init(render_logger, video_renderer)) ==
               NULL) {
        LOGE("Could not init audio renderer");
        return -1;
    }

    if (video_renderer) video_renderer_start(video_renderer);
    if (audio_renderer) audio_renderer_start(audio_renderer);

    /* write desired display pixel width, pixel height to raop (use 0 for default values) */
    raop_set_display_size(raop, display_size[0], display_size[1]);

    /* network port selection (ports listed as "0" will be dynamically assigned) */
    raop_set_tcp_ports(raop, tcp);
    raop_set_udp_ports(raop, udp);

    unsigned short port = raop_get_port(raop);
    raop_start(raop, &port);
    raop_set_port(raop, port);

    int error;
    dnssd = dnssd_init(name.c_str(), strlen(name.c_str()), hw_addr.data(), hw_addr.size(), &error);
    if (error) {
        LOGE("Could not initialize dnssd library!");
        return -2;
    }

    raop_set_dnssd(raop, dnssd);

    dnssd_register_raop(dnssd, port);
    dnssd_register_airplay(dnssd, port + 1);

    return 0;
}

int stop_server () {
    raop_destroy(raop);
    dnssd_unregister_raop(dnssd);
    dnssd_unregister_airplay(dnssd);
    if (audio_renderer) audio_renderer_destroy(audio_renderer);
    video_renderer_destroy(video_renderer);
    return 0;
}
