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

#define VERSION "1.31"

#define DEFAULT_NAME "UxPlay"
#define DEFAULT_DEBUG_LOG false
#define LOWEST_ALLOWED_PORT 1024
#define HIGHEST_PORT 65535


static int start_server (std::vector<char> hw_addr, std::string name, unsigned short display[4],
                 unsigned short tcp[2], unsigned short udp[3], videoflip_t videoflip[2],
                 bool use_audio,  bool debug_log);

static int stop_server ();

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

static std::string find_mac () {
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
static std::string random_mac () {
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

static void print_info (char *name) {
    printf("UxPlay %s: An open-source AirPlay mirroring server based on RPiPlay\n", VERSION);
    printf("Usage: %s [-n name] [-s wxh] [-p [n]]\n", name);
    printf("Options:\n");
    printf("-n name   Specify the network name of the AirPlay server\n");
    printf("-s wxh[@r]Set display resolution [refresh_rate] default 1920x1080[@60]\n");
    printf("-fps n    Set maximum allowed streaming framerate, default 30\n");
    printf("-f {H|V|I}Horizontal|Vertical flip, or both=Inversion=rotate 180 deg\n");
    printf("-r {R|L}  rotate 90 degrees Right (cw) or Left (ccw)\n");
    printf("-p n      Use fixed UDP+TCP network ports n:n+1:n+2. (n>1023)\n");
    printf("-p        Use legacy UDP 6000:6001:7011 TCP 7000:7001:7100\n");
    printf("-m        use random MAC address (use for concurrent UxPlay's)\n");
    printf("-a        Turn audio off. video output only\n");
    printf("-d        Enable debug logging\n");
    printf("-v/-h     Displays this help and version information\n");
}

bool option_has_value(const int i, const int argc, const char *arg, const char *next_arg) {
    if (i >= argc - 1 || next_arg[0] == '-') {
        fprintf(stderr,"invalid \"%s\" had no argument\n", arg);
        return false;
     }
    return true;
}

static bool get_display_settings (std::string value, unsigned short *w, unsigned short *h, unsigned short *r) {
    // assume str  = wxh@r is valid if w and h are positive decimal integers
    // with no more than 4 digits, r < 256 (stored in one byte).
    char *end;
    std::size_t pos = value.find_first_of("x");
    if (!pos) return false;
    std::string str1 = value.substr(pos+1);
    value.erase(pos);
    if (value.length() == 0 || value.length() > 4 || value[0] == '-') return false;
    *w = (unsigned short) strtoul(value.c_str(), &end, 10);
    if (*end || *w == 0)  return false;
    pos = str1.find_first_of("@");
    if(pos) {
        std::string str2 = str1.substr(pos+1);
        if (str2.length() == 0 || str2.length() > 3 || str2[0] == '-') return false;
        *r = (unsigned short) strtoul(str2.c_str(), &end, 10);
        if (*end || *r == 0 || *r > 255) return false;
        str1.erase(pos);
    }
    if (str1.length() == 0 || str1.length() > 4 || str1[0] == '-') return false;
    *h = (unsigned short) strtoul(str1.c_str(), &end, 10);
    if (*end || *h == 0) return false;
    return true;
}

static bool get_fps (const char *str, unsigned short *n) {
    // str must be a positive decimal integer < 256 (stored in one byte)
    char *end;
    if (strlen(str) == 0 || strlen(str) > 3 || str[0] == '-') return false;
    *n = (unsigned short) strtoul(str, &end, 10);
    if (*end || *n == 0 || *n > 255) return false;
    return true;
}

static bool get_lowest_port (const char *str, unsigned short *n) {
    //str must be a positive decimal integer in allowed range, 5 digits max.
    char *end;
    unsigned long l;
    if (strlen(str)  == 0 || strlen(str) > 5 || str[0] == '-') return false;
    *n = (unsigned short) (l = strtoul(str, &end, 10));
    if (*end) return false;
    if (l  < LOWEST_ALLOWED_PORT || l > HIGHEST_PORT - 2 ) return false;
    return true;
}

static bool get_videoflip (const char *str, videoflip_t *videoflip) {
    if (strlen(str) > 1) return false;
    switch (str[0]) {
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

static bool get_videorotate (const char *str, videoflip_t *videoflip) {
    if (strlen(str) > 1) return false;
    switch (str[0]) {
    case 'L':
        *videoflip = LEFT;
        break;
    case 'R':
        *videoflip = RIGHT;
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
    unsigned short display[4] = {0}, tcp[2] = {0}, udp[3] = {0};
    videoflip_t videoflip[2] = { NONE , NONE };
    
#ifdef SUPPRESS_AVAHI_COMPAT_WARNING
    // suppress avahi_compat nag message.  avahi emits a "nag" warning (once)
    // if  getenv("AVAHI_COMPAT_NOWARN") returns null.
    static char avahi_compat_nowarn[] = "AVAHI_COMPAT_NOWARN=1";
    if (!getenv("AVAHI_COMPAT_NOWARN")) putenv(avahi_compat_nowarn);
#endif
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "-n") {
            if (!option_has_value(i, argc, argv[i], argv[i+1])) exit(1);
            server_name = std::string(argv[++i]);
        } else if (arg == "-s") {
            if (!option_has_value(i, argc, argv[i], argv[i+1])) exit(1);
            std::string value(argv[++i]);
            if (!get_display_settings(value, &display[0], &display[1], &display[2])) {
                fprintf(stderr, "invalid \"-s %s\"; -s wxh : max w,h=9999; -s wxh@r : max r=255\n",
                        argv[i]);
                exit(1);
            }
        } else if (arg == "-fps") {
            if (!option_has_value(i, argc, argv[i], argv[i+1])) exit(1);
            if (!get_fps(argv[++i], &display[3])) {
                fprintf(stderr, "invalid \"-fps %s\"; -fps n : max n=255, default n=30\n", argv[i]);
                exit(1);
            }
        } else if (arg == "-f") {
            if (!option_has_value(i, argc, argv[i], argv[i+1])) exit(1);
            if (!get_videoflip(argv[++i], &videoflip[0])) {
                fprintf(stderr,"invalid \"-f %s\" , unknown flip type, choices are H, V, I\n",argv[i]);
                exit(1);
            }
        } else if (arg == "-r") {
            if (!option_has_value(i, argc, argv[i], argv[i+1])) exit(1);
            if (!get_videorotate(argv[++i], &videoflip[1])) {
                fprintf(stderr,"invalid \"-r %s\" , unknown rotation  type, choices are R, L\n",argv[i]);
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
        } else if (arg == "-m") {
            use_random_hw_addr  = true;
        } else if (arg == "-a") {
            use_audio = false;
        } else if (arg == "-d") {
            debug_log = !debug_log;
        } else if (arg == "-h" || arg == "-v") {
            print_info(argv[0]);
            exit(0);
        } else {
            LOGE("unknown option %s, stopping\n",argv[i]);
            exit(1);
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

    relaunch:
    if (start_server(server_hw_addr, server_name, display, tcp, udp,
                     videoflip,use_audio, debug_log) != 0) {
        return 1;
    }
    running = true;
    while (running) {
        if ((video_renderer_listen(video_renderer))) {
            stop_server();
            goto relaunch;
        }
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

int start_server (std::vector<char> hw_addr, std::string name, unsigned short display[4],
                 unsigned short tcp[2], unsigned short udp[3], videoflip_t videoflip[2],
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

    /* write desired display pixel width, pixel height, refresh_rate, */
    /* and  max_fps to raop (use 0 for default values)                */
    raop_set_display(raop, display[0], display[1], display[2], display[3]);

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
