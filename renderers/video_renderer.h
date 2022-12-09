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

/* 
 * H264 renderer using gstreamer
*/

#ifndef VIDEO_RENDERER_H
#define VIDEO_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../lib/logger.h"
#include "../lib/raop_ntp.h"

typedef enum videoflip_e {
    NONE,
    LEFT,
    RIGHT,
    INVERT,
    VFLIP,
    HFLIP,
} videoflip_t;

typedef struct video_renderer_s video_renderer_t;

void video_renderer_init (logger_t *logger, const char *server_name, videoflip_t videoflip[2], const char *parser,
                          const char *decoder, const char *converter, const char *videosink, const bool *fullscreen);
void video_renderer_start ();
void video_renderer_stop ();
void video_renderer_render_buffer (raop_ntp_t *ntp, unsigned char* data, int data_len, uint64_t pts, int nal_count);
void video_renderer_flush ();
unsigned int video_renderer_listen(void *loop);
void video_renderer_destroy ();
void video_renderer_size(float *width_source, float *height_source, float *width, float *height);
  
  /* not implemented for gstreamer */
void video_renderer_update_background (int type); 

#ifdef __cplusplus
}
#endif

#endif //VIDEO_RENDERER_H

