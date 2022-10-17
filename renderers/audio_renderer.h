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

#ifndef AUDIO_RENDERER_H
#define AUDIO_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../lib/raop_ntp.h"

bool gstreamer_init();
void audio_renderer_init(logger_t *logger, const char* audiosink);
void audio_renderer_start(unsigned char* compression_type);
void audio_renderer_stop();
void audio_renderer_render_buffer(raop_ntp_t *ntp, unsigned char* data, int data_len,
                                  uint64_t ntp_time, uint64_t rtp_time, unsigned short seqnum);
void audio_renderer_set_volume(float volume);
void audio_renderer_flush();
void audio_renderer_destroy();

#ifdef __cplusplus
}
#endif

#endif //AUDIO_RENDERER_H
