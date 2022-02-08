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
#include <sys/utsname.h>
#include <glib-unix.h>
#include <assert.h>

#include <sys/socket.h>
#include <ifaddrs.h>
#ifdef __linux__
#include <netpacket/packet.h>
#else
#include <net/if_dl.h>
#endif

#include "log.h"
#include "lib/raop.h"
#include "lib/stream.h"
#include "lib/logger.h"
#include "lib/dnssd.h"
#include "renderers/video_renderer.h"
#include "renderers/audio_renderer.h"

#define VERSION "1.47"

#define DEFAULT_NAME "UxPlay"
#define DEFAULT_DEBUG_LOG false
#define LOWEST_ALLOWED_PORT 1024
#define HIGHEST_PORT 65535
#define NTP_TIMEOUT_LIMIT 10

static std::string server_name = DEFAULT_NAME;
static int start_raop_server (std::vector<char> hw_addr, std::string name, unsigned short display[5],
                 unsigned short tcp[3], unsigned short udp[3], bool debug_log);
static int stop_raop_server ();
extern "C" void log_callback (void *cls, int level, const char *msg) ;

static dnssd_t *dnssd = NULL;
static raop_t *raop = NULL;
static logger_t *render_logger = NULL;

static bool relaunch_video = false;
static bool relaunch_server = false;
static bool reset_loop = false;
static uint open_connections = 0;
static bool connections_stopped = false;
static unsigned int server_timeout = 0;
static unsigned int counter;
static std::string videosink = "autovideosink";
static videoflip_t videoflip[2] = { NONE , NONE };
static bool use_video = true;
static unsigned char compression_type = 0;
static std::string audiosink = "autoaudiosink";
static bool use_audio = true;
static bool new_window_closing_behavior = true;
static bool close_window;
static std::string decoder = "decodebin";
static bool show_client_FPS_data = false;
static unsigned int max_ntp_timeouts = NTP_TIMEOUT_LIMIT;

static gboolean connection_callback (gpointer loop){
    if (!connections_stopped) {
        counter = 0;
    } else {
        if (++counter == server_timeout) {
            LOGD("no connections for %d seconds: relaunch server",server_timeout);
            relaunch_server = true;
            relaunch_video = false;
            g_main_loop_quit((GMainLoop *) loop);
        }
    }
    return TRUE;
}

static gboolean reset_callback(gpointer loop) {
    if (reset_loop) {
        g_main_loop_quit((GMainLoop *) loop);
    }
    return TRUE;
}

static gboolean  sigint_callback(gpointer loop) {
    relaunch_video = false;
    relaunch_server = false;
    g_main_loop_quit((GMainLoop *) loop);
    return TRUE;
}

static gboolean  sigterm_callback(gpointer loop) {
    relaunch_video = false;
    relaunch_server = false;
    g_main_loop_quit((GMainLoop *) loop);
    return TRUE;
}

static void main_loop()  {
    guint connection_watch_id = 0;
    guint gst_bus_watch_id = 0;
    GMainLoop *loop = g_main_loop_new(NULL,FALSE);
    if (server_timeout) {
        connection_watch_id = g_timeout_add_seconds(1, (GSourceFunc) connection_callback, (gpointer) loop);
    }
    relaunch_video = false;
    relaunch_server = false;
    if (use_video) {
        relaunch_video = true;
        gst_bus_watch_id = (guint) video_renderer_listen((void *)loop);
    }
    guint reset_watch_id = g_timeout_add(100, (GSourceFunc) reset_callback, (gpointer) loop);
    guint sigterm_watch_id = g_unix_signal_add(SIGTERM, (GSourceFunc) sigterm_callback, (gpointer) loop);
    guint sigint_watch_id = g_unix_signal_add(SIGINT, (GSourceFunc) sigint_callback, (gpointer) loop);
    g_main_loop_run(loop);

    if (gst_bus_watch_id > 0) g_source_remove(gst_bus_watch_id);
    if (sigint_watch_id > 0) g_source_remove(sigint_watch_id);
    if (sigterm_watch_id > 0) g_source_remove(sigterm_watch_id);
    if (reset_watch_id > 0) g_source_remove(reset_watch_id);
    if (connection_watch_id > 0) g_source_remove(connection_watch_id);
    g_main_loop_unref(loop);
}    

static int parse_hw_addr (std::string str, std::vector<char> &hw_addr) {
    for (int i = 0; i < str.length(); i += 3) {
        hw_addr.push_back((char) stol(str.substr(i), NULL, 16));
    }
    return 0;
}

static std::string find_mac () {
/*  finds the MAC address of a network interface *
 *  in a Linux, *BSD or macOS system.            */
    std::string mac = "";
    struct ifaddrs *ifap, *ifaptr;
    int non_null_octets = 0;
    unsigned char octet[6], *ptr;
    if (getifaddrs(&ifap) == 0) {
        for(ifaptr = ifap; ifaptr != NULL; ifaptr = ifaptr->ifa_next) {
            if(ifaptr->ifa_addr == NULL) continue;
#ifdef __linux__
            if (ifaptr->ifa_addr->sa_family != AF_PACKET) continue;
            struct sockaddr_ll *s = (struct sockaddr_ll*) ifaptr->ifa_addr;
            for (int i = 0; i < 6; i++) {
                if ((octet[i] = s->sll_addr[i]) != 0) non_null_octets++;
            }
#else    /* macOS and *BSD */
            if (ifaptr->ifa_addr->sa_family != AF_LINK) continue;
            ptr = (unsigned char *) LLADDR((struct sockaddr_dl *) ifaptr->ifa_addr);
            for (int i= 0; i < 6 ; i++) {
                if ((octet[i] = *ptr) != 0) non_null_octets++;
                ptr++;
            }
#endif
            if (non_null_octets) {
                mac.erase();
                char str[3];
                for (int i = 0; i < 6 ; i++) {
                    sprintf(str,"%02x", octet[i]);
                    mac = mac + str;
                    if (i < 5) mac = mac + ":";
                }
                break;
            }
        }
    }
    freeifaddrs(ifap);
    return mac;
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
    printf("-nh       Do not add \"@hostname\" at the end of the AirPlay server name\n");
    printf("-s wxh[@r]Set display resolution [refresh_rate] default 1920x1080[@60]\n");
    printf("-o        Set mirror \"overscanned\" mode on (not usually needed)\n");
    printf("-fps n    Set maximum allowed streaming framerate, default 30\n");
    printf("-f {H|V|I}Horizontal|Vertical flip, or both=Inversion=rotate 180 deg\n");
    printf("-r {R|L}  Rotate 90 degrees Right (cw) or Left (ccw)\n");
    printf("-p        Use legacy ports UDP 6000:6001:7011 TCP 7000:7001:7100\n");
    printf("-p n      Use TCP and UDP ports n,n+1,n+2. range %d-%d\n", LOWEST_ALLOWED_PORT, HIGHEST_PORT);
    printf("          use \"-p n1,n2,n3\" to set each port, \"n1,n2\" for n3 = n2+1\n");
    printf("          \"-p tcp n\" or \"-p udp n\" sets TCP or UDP ports separately\n");
    printf("-m        Use random MAC address (use for concurrent UxPlay's)\n");
    printf("-t n      Relaunch server if no connection existed in last n seconds\n");
    printf("-vs       Choose the GStreamer videosink; default \"autovideosink\"\n");
    printf("          some choices: ximagesink,xvimagesink,vaapisink,glimagesink,\n");
    printf("          gtksink,waylandsink,osximagesink,fpsdisplaysink, etc.\n");
    printf("-vs 0     Streamed audio only, with no video display window\n");
    printf("-avdec    Force software h264 video decoding with libav h264 decoder\n"); 
    printf("-as       Choose the GStreamer audiosink; default \"autoaudiosink\"\n");
    printf("          choices: pulsesink,alsasink,osssink,oss4sink,osxaudiosink,etc.\n");
    printf("-as 0     (or -a)  Turn audio off, streamed video only\n");
    printf("-reset n  Reset after 3n seconds client silence (default %d, 0 = never)\n", NTP_TIMEOUT_LIMIT);
    printf("-nc       do Not Close video window when client stops mirroring\n");  
    printf("-FPSdata  Show video-streaming performance reports sent by client.\n");
    printf("-d        Enable debug logging\n");
    printf("-v or -h  Displays this help and version information\n");
}

bool option_has_value(const int i, const int argc, std::string option, const char *next_arg) {
    if (i >= argc - 1 || next_arg[0] == '-') {
        LOGE("invalid: \"%s\" had no argument", option.c_str());
        return false;
     }
    return true;
}

static bool get_display_settings (std::string value, unsigned short *w, unsigned short *h, unsigned short *r) {
    // assume str  = wxh@r is valid if w and h are positive decimal integers
    // with no more than 4 digits, r < 256 (stored in one byte).
    char *end;
    std::size_t pos = value.find_first_of("x");
    if (pos == std::string::npos) return false;
    std::string str1 = value.substr(pos+1);
    value.erase(pos);
    if (value.length() == 0 || value.length() > 4 || value[0] == '-') return false;
    *w = (unsigned short) strtoul(value.c_str(), &end, 10);
    if (*end || *w == 0)  return false;
    pos = str1.find_first_of("@");
    if(pos != std::string::npos) {
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

static bool get_value (const char *str, unsigned int *n) {
    // if n > 0 str must be a positive decimal <= input value *n  
    // if n = 0, str must be a non-negative decimal
    if (strlen(str) == 0 || strlen(str) > 10 || str[0] == '-') return false;
    char *end;
    unsigned long l = strtoul(str, &end, 10);
    if (*end) return false;
    if (*n && (l == 0 || l > *n)) return false;
    *n = (unsigned int) l;
    return true;
}

static bool get_ports (int nports, std::string option, const char * value, unsigned short * const port) {
    /*valid entries are comma-separated values port_1,port_2,...,port_r, 0 < r <= nports */
    /*where ports are distinct, and are in the allowed range.                            */
    /*missing values are consecutive to last given value (at least one value needed).    */
    char *end;
    unsigned long l;
    std::size_t pos;
    std::string val(value), str;
    for (int i = 0; i <= nports ; i++)  {
        if(i == nports) break;
        pos = val.find_first_of(',');
        str = val.substr(0,pos);
        if(str.length() == 0 || str.length() > 5 || str[0] == '-') break;
        l = strtoul(str.c_str(), &end, 10);
        if (*end || l < LOWEST_ALLOWED_PORT || l > HIGHEST_PORT) break;
         *(port + i) = (unsigned short) l;
        for  (int j = 0; j < i ; j++) {
            if( *(port + j) == *(port + i)) break;
        }
        if(pos == std::string::npos) {
            if (nports + *(port + i) > i + 1 + HIGHEST_PORT) break;
            for (int j = i + 1; j < nports; j++) {
                *(port + j) = *(port + j - 1) + 1;
            }
            return true;
        }
        val.erase(0, pos+1);
    }
    LOGE("invalid \"%s %s\", all %d ports must be in range [%d,%d]",
         option.c_str(), value, nports, LOWEST_ALLOWED_PORT, HIGHEST_PORT);
    return false;
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

static void append_hostname(std::string &server_name) {
    struct utsname buf;
    if (!uname(&buf)) {
      std::string name = server_name;
      name.append("@");
      name.append(buf.nodename);
      server_name = name;
    }
}

int main (int argc, char *argv[]) {
    std::vector<char> server_hw_addr;
    bool do_append_hostname = true;
    bool use_random_hw_addr = false;
    bool debug_log = DEFAULT_DEBUG_LOG;
    unsigned short display[5] = {0}, tcp[3] = {0}, udp[3] = {0};

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
            if (!option_has_value(i, argc, arg, argv[i+1])) exit(1);
            server_name = std::string(argv[++i]);
        } else if (arg == "-nh") {
            do_append_hostname = false;
        } else if (arg == "-s") {
            if (!option_has_value(i, argc, argv[i], argv[i+1])) exit(1);
            std::string value(argv[++i]);
            if (!get_display_settings(value, &display[0], &display[1], &display[2])) {
                fprintf(stderr, "invalid \"-s %s\"; -s wxh : max w,h=9999; -s wxh@r : max r=255\n",
                        argv[i]);
                exit(1);
            }
        } else if (arg == "-fps") {
            if (!option_has_value(i, argc, arg, argv[i+1])) exit(1);
            unsigned int n = 255;
            if (!get_value(argv[++i], &n)) {
                fprintf(stderr, "invalid \"-fps %s\"; -fps n : max n=255, default n=30\n", argv[i]);
                exit(1);
            }
            display[3] = (unsigned short) n;
        } else if (arg == "-o") {
            display[4] = 1;
        } else if (arg == "-f") {
            if (!option_has_value(i, argc, arg, argv[i+1])) exit(1);
            if (!get_videoflip(argv[++i], &videoflip[0])) {
                fprintf(stderr,"invalid \"-f %s\" , unknown flip type, choices are H, V, I\n",argv[i]);
                exit(1);
            }
        } else if (arg == "-r") {
            if (!option_has_value(i, argc, arg, argv[i+1])) exit(1);
            if (!get_videorotate(argv[++i], &videoflip[1])) {
                fprintf(stderr,"invalid \"-r %s\" , unknown rotation  type, choices are R, L\n",argv[i]);
                exit(1);
            }
        } else if (arg == "-p") {
            if (i == argc - 1 || argv[i + 1][0] == '-') {
                tcp[0] = 7100; tcp[1] = 7000; tcp[2] = 7001;
                udp[0] = 7011; udp[1] = 6001; udp[2] = 6000;
                continue;
            }
            std::string value(argv[++i]);
            if (value == "tcp") {
                arg.append(" tcp");
                if(!get_ports(3, arg, argv[++i], tcp)) exit(1);
            } else if (value == "udp") {
                arg.append( " udp");
                if(!get_ports(3, arg, argv[++i], udp)) exit(1);
            } else {
                if(!get_ports(3, arg, argv[i], tcp)) exit(1);
                for (int j = 1; j < 3; j++) {
                    udp[j] = tcp[j];
                }
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
        } else if (arg == "-vs") {
            if (!option_has_value(i, argc, arg, argv[i+1])) exit(1);
            videosink.erase();
            videosink.append(argv[++i]);
        } else if (arg == "-as") {
            if (!option_has_value(i, argc, arg, argv[i+1])) exit(1);
            audiosink.erase();
            audiosink.append(argv[++i]);
        } else if (arg == "-t") {
            if (!option_has_value(i, argc, argv[i], argv[i+1])) exit(1);
            server_timeout = 0;
            bool valid = get_value(argv[++i], &server_timeout);
            if (!valid || server_timeout == 0) {
                fprintf(stderr,"invalid \"-t %s\", must have -t n with  n > 0\n",argv[i]);
                exit(1);
            }
        } else if (arg == "-nc") {
            new_window_closing_behavior = false;
        } else if (arg == "-avdec") {
            decoder.erase();
            decoder = "h264parse ! avdec_h264";
        } else if (arg == "-FPSdata") {
            show_client_FPS_data = true;
        } else if (arg == "-reset") {
            max_ntp_timeouts = 0;
            if (!get_value(argv[++i], &max_ntp_timeouts)) {
                fprintf(stderr, "invalid \"-reset %s\"; -reset n must have n >= 0,  default n = %d\n", argv[i], NTP_TIMEOUT_LIMIT);
                exit(1);
            }      
        } else {
            LOGE("unknown option %s, stopping\n",argv[i]);
            exit(1);
        }
    }

    if (audiosink == "0") {
        use_audio = false;
    }

#if __APPLE__
    /* force use of -nc option on macOS */
    LOGI("macOS detected: use -nc option as workaround for GStreamer problem");
    new_window_closing_behavior = false;
    server_timeout = 0;
#endif

    if (videosink == "0") {
        use_video = false;
	videosink.erase();
        videosink.append("fakesink");
	LOGI("video_disabled");
        display[3] = 1; /* set fps to 1 frame per sec when no video will be shown */
    }

    if (do_append_hostname) append_hostname(server_name);
    
    render_logger = logger_init();
    logger_set_callback(render_logger, log_callback, NULL);
    logger_set_level(render_logger, debug_log ? LOGGER_DEBUG : LOGGER_INFO);

    if (use_audio) {
        audio_renderer_init(render_logger, audiosink.c_str());
    } else {
        LOGI("audio_disabled");
    }

    if (use_video) {
        video_renderer_init(render_logger, server_name.c_str(), videoflip, decoder.c_str(), videosink.c_str());
        video_renderer_start();
    }
    
    if (udp[0]) LOGI("using network ports UDP %d %d %d TCP %d %d %d",
                      udp[0],udp[1], udp[2], tcp[0], tcp[1], tcp[2]);

    std::string mac_address;
    if (!use_random_hw_addr) mac_address = find_mac();
    if (mac_address.empty()) {
        srand(time(NULL) * getpid());
        mac_address = random_mac();
        LOGI("using randomly-generated MAC address %s",mac_address.c_str());
    } else {
        LOGI("using system MAC address %s",mac_address.c_str());
    }
    parse_hw_addr(mac_address, server_hw_addr);
    mac_address.clear();

    connections_stopped = true;
    relaunch:
    if (start_raop_server(server_hw_addr, server_name, display, tcp, udp, debug_log)) {
        return 1;
    }
    reconnect:
    counter = 0;
    compression_type = 0;
    close_window = new_window_closing_behavior; 
    main_loop();
    if (relaunch_server || relaunch_video || reset_loop) {
        if(reset_loop) {
            reset_loop = false;
        } else {
            raop_stop(raop);
        }
        if (use_audio) audio_renderer_stop();
        if (use_video && close_window) {
            video_renderer_destroy();
            video_renderer_init(render_logger, server_name.c_str(), videoflip, decoder.c_str(), videosink.c_str());
            video_renderer_start();
        }
        if (reset_loop) goto reconnect;
        if (relaunch_video) {
            unsigned short port = raop_get_port(raop);
            raop_start(raop, &port);
            raop_set_port(raop, port);
            goto reconnect;
        } else {
            LOGI("Re-launching RAOP server...");
            stop_raop_server();
            goto relaunch;
        }
    } else {
        LOGI("Stopping...");
        stop_raop_server();
    }
    if (use_audio) {
      audio_renderer_destroy();
    }
    if (use_video)  {
        video_renderer_destroy();
    }
    logger_destroy(render_logger);
}

// Server callbacks
extern "C" void conn_init (void *cls) {
    open_connections++;
    connections_stopped = false;
    LOGI("Open connections: %i", open_connections);
    //video_renderer_update_background(1);
}

extern "C" void conn_destroy (void *cls) {
    //video_renderer_update_background(-1);
    open_connections--;
    LOGI("Open connections: %i", open_connections);
    if(!open_connections) {
        connections_stopped = true;
    }
}

extern "C" void conn_reset (void *cls) {
    LOGI("***ERROR lost connection with client");
    close_window = false;    /* leave "frozen" window open */
    raop_stop(raop);
    reset_loop = true;
}

extern "C" void conn_teardown(void *cls, bool *teardown_96, bool *teardown_110) {
    if (*teardown_110 && close_window) {
        reset_loop = true;
    }
}

extern "C" void audio_process (void *cls, raop_ntp_t *ntp, aac_decode_struct *data) {
    if (use_audio) {
        audio_renderer_render_buffer(ntp, data->data, data->data_len, data->pts);
    }
}

extern "C" void video_process (void *cls, raop_ntp_t *ntp, h264_decode_struct *data) {
    if (use_video) {
        video_renderer_render_buffer(ntp, data->data, data->data_len, data->pts, data->frame_type);
    }
}

extern "C" void audio_flush (void *cls) {
  if (use_audio) {
      audio_renderer_flush();
  }
}

extern "C" void video_flush (void *cls) {
    if (use_video) {
        video_renderer_flush();
    }
}

extern "C" void audio_set_volume (void *cls, float volume) {
    if (use_audio) {
        audio_renderer_set_volume(volume);
    }
}

extern "C" void audio_get_format (void *cls, unsigned char *ct, unsigned short *spf, bool *usingScreen, bool *isMedia, uint64_t *audioFormat) {
    LOGI("ct=%d spf=%d usingScreen=%d isMedia=%d  audioFormat=0x%lx",*ct, *spf, *usingScreen, *isMedia, (unsigned long) *audioFormat);
    if (use_audio) {
        audio_renderer_start(ct);
    }
}

extern "C" void video_report_size(void *cls, float *width_source, float *height_source, float *width, float *height) {
    video_renderer_size(width_source, height_source, width, height);
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

int start_raop_server (std::vector<char> hw_addr, std::string name, unsigned short display[5],
                  unsigned short tcp[3], unsigned short udp[3], bool debug_log) {
    raop_callbacks_t raop_cbs;
    memset(&raop_cbs, 0, sizeof(raop_cbs));
    raop_cbs.conn_init = conn_init;
    raop_cbs.conn_destroy = conn_destroy;
    raop_cbs.conn_reset = conn_reset;
    raop_cbs.conn_teardown = conn_teardown;
    raop_cbs.audio_process = audio_process;
    raop_cbs.video_process = video_process;
    raop_cbs.audio_flush = audio_flush;
    raop_cbs.video_flush = video_flush;
    raop_cbs.audio_set_volume = audio_set_volume;
    raop_cbs.audio_get_format = audio_get_format;
    raop_cbs.video_report_size = video_report_size;

    /* set max number of connections = 2 */
    raop = raop_init(2, &raop_cbs);
    if (raop == NULL) {
        LOGE("Error initializing raop!");
        return -1;
    }

    /* write desired display pixel width, pixel height, refresh_rate, max_fps, overscanned.  */
    /* use 0 for default values 1920,1080,60,30,0; these are sent to the Airplay client      */

    if (display[0]) raop_set_plist(raop, "width", (int) display[0]);
    if (display[1]) raop_set_plist(raop, "height", (int) display[1]);
    if (display[2]) raop_set_plist(raop, "refreshRate", (int) display[2]);
    if (display[3]) raop_set_plist(raop, "maxFPS", (int) display[3]);
    if (display[4]) raop_set_plist(raop, "overscanned", (int) display[4]);
 
    if (show_client_FPS_data) raop_set_plist(raop, "clientFPSdata", 1);
    raop_set_plist(raop, "max_ntp_timeouts", max_ntp_timeouts);

    /* network port selection (ports listed as "0" will be dynamically assigned) */
    raop_set_tcp_ports(raop, tcp);
    raop_set_udp_ports(raop, udp);
    
    raop_set_log_callback(raop, log_callback, NULL);
    raop_set_log_level(raop, debug_log ? RAOP_LOG_DEBUG : LOGGER_INFO);

    unsigned short port = raop_get_port(raop);
    raop_start(raop, &port);
    raop_set_port(raop, port);

    int error;
    dnssd = dnssd_init(name.c_str(), strlen(name.c_str()), hw_addr.data(), hw_addr.size(), &error);
    if (error) {
        LOGE("Could not initialize dnssd library!");
        stop_raop_server();
        return -2;
    }

    raop_set_dnssd(raop, dnssd);

    dnssd_register_raop(dnssd, port);
    if (tcp[2]) {
        port = tcp[2];
    } else {
      port = (port != HIGHEST_PORT ? port + 1 : port - 1);
    }
    dnssd_register_airplay(dnssd, port);

    return 0;
}

int stop_raop_server () {
    if (raop) raop_destroy(raop);
    if (dnssd) {
        dnssd_unregister_raop(dnssd);
        dnssd_unregister_airplay(dnssd);
        dnssd_destroy(dnssd);
    }
    return 0;
}
