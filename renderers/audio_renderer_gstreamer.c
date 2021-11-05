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

#include "audio_renderer.h"
#include <assert.h>
#include <math.h>
#include <gst/app/gstappsrc.h>

/* GStreamer Caps strings for Airplay-defined connection types (ct) */

/* ct = 1; linear PCM (uncompressed): 44100/16/2, S16LE */
static const char lpcm[]="audio/x-raw,rate=(int)44100,channels=(int)2,format=S16LE,layout=interleaved";

/* ct = 2; codec_data is ALAC magic cookie:  44100/16/2 spf = 352 */    
static const char alac[] = "audio/x-alac,mpegversion=(int)4,channnels=(int)2,rate=(int)44100,stream-format=raw,codec_data=(buffer)"
                           "00000024""616c6163""00000000""00000160""0010280a""0e0200ff""00000000""00000000""0000ac44";

/* ct = 4; codec_data from MPEG v4 ISO 14996-3 Section 1.6.2.1:  AAC-LC 44100/2 spf = 1024 */
static const char aac_lc[] ="audio/mpeg,mpegversion=(int)4,channnels=(int)2,rate=(int)44100,stream-format=raw,codec_data=(buffer)1210";

/* ct = 8; codec_data from MPEG v4 ISO 14996-3 Section 1.6.2.1: AAC_ELD 44100/2  spf = 460 */
static const char aac_eld[] ="audio/mpeg,mpegversion=(int)4,channnels=(int)2,rate=(int)44100,stream-format=raw,codec_data=(buffer)f8e85000";

struct audio_renderer_s {
    logger_t *logger;
    GstElement *appsrc; 
    GstElement *pipeline;
    GstElement *volume;
};

static gboolean check_plugins (void)
{
  int i;
  gboolean ret;
  GstRegistry *registry;
  const gchar *needed[] = {"app", "libav", "playback", "autodetect", NULL};

  registry = gst_registry_get ();
  ret = TRUE;
  for (i = 0; i < g_strv_length ((gchar **) needed); i++) {
    GstPlugin *plugin;
    plugin = gst_registry_find_plugin (registry, needed[i]);
    if (!plugin) {
      g_print ("Required gstreamer plugin '%s' not found\n", needed[i]);
      ret = FALSE;
      continue;
    }
    gst_object_unref (plugin);
  }
  return ret;
}

audio_renderer_t *audio_renderer_init(logger_t *logger, unsigned char *compression_type, const char* audiosink) {
    audio_renderer_t *renderer;
    GError *error = NULL;
    GstCaps *caps = NULL;




    switch (*compression_type) {
    case 1:    /* uncompressed PCM */
    case 2:    /* Apple lossless ALAC */
    case 4:    /* AAC-LC */
    case 8:    /* AAC-ELD */
      logger_log(logger, LOGGER_INFO , "audio_renderer_init: compression_type ct = %d", *compression_type);
      break;
    default:
        logger_log(logger, LOGGER_ERR, "audio_renderer_init: unsupported compression_type ct = %d", *compression_type);
      return NULL;
    }

    
    renderer = calloc(1, sizeof(audio_renderer_t));
    if (!renderer) {
        return NULL;
    }
    renderer->logger = logger;

    assert(check_plugins ());

    GString *launch = g_string_new("appsrc name=audio_source stream-type=0 format=GST_FORMAT_TIME is-live=true ! queue ! ");
    if (*compression_type == 8 || *compression_type == 4) {
        g_string_append(launch, "avdec_aac ! ");
    } else if (*compression_type == 2) {
        g_string_append(launch, "avdec_alac ! ");
    }
    g_string_append(launch, "audioconvert ! volume name=volume ! level ! ");
    g_string_append(launch, audiosink);
    g_string_append(launch, " sync=false");
    renderer->pipeline = gst_parse_launch(launch->str,  &error);
    g_assert (renderer->pipeline);
    g_string_free(launch, TRUE);

    renderer->appsrc = gst_bin_get_by_name (GST_BIN (renderer->pipeline), "audio_source");
    renderer->volume = gst_bin_get_by_name (GST_BIN (renderer->pipeline), "volume");

    if (*compression_type == 8) {
        logger_log(logger, LOGGER_INFO, "AAC-ELD 44100/2");
        caps =  gst_caps_from_string(aac_eld);
    } else if (*compression_type == 2) {
        logger_log(logger, LOGGER_INFO, "ALAC 44100/16/2");
        caps = gst_caps_from_string(alac);;
    } else if (*compression_type == 4) {
        logger_log(logger, LOGGER_INFO, "AAC-LC 44100/2");
        caps = gst_caps_from_string(aac_lc);
        logger_log(logger, LOGGER_INFO, "uncompressed PCM 44100/16/2");
    } else if (*compression_type == 1) {
        caps = gst_caps_from_string(lpcm);
    }

    g_object_set(renderer->appsrc, "caps", caps, NULL);
    gst_caps_unref(caps);
    
    return renderer;
}

void audio_renderer_start(audio_renderer_t *renderer) {
    //g_signal_connect( renderer->pipeline, "deep-notify", G_CALLBACK(gst_object_default_deep_notify ), NULL );
    gst_element_set_state (renderer->pipeline, GST_STATE_PLAYING);
}

void audio_renderer_render_buffer(audio_renderer_t *renderer, raop_ntp_t *ntp, unsigned char* data, int data_len, uint64_t pts) {

    GstBuffer *buffer;

    if (data_len == 0) return;

    buffer = gst_buffer_new_and_alloc(data_len);
    assert(buffer != NULL);
    GST_BUFFER_DTS(buffer) = (GstClockTime)pts;
    gst_buffer_fill(buffer, 0, data, data_len);
    gst_app_src_push_buffer(GST_APP_SRC(renderer->appsrc), buffer);

}

void audio_renderer_set_volume(audio_renderer_t *renderer, float volume) {
    float avol;
        if (fabs(volume) < 28) {
	        avol=floorf(((28-fabs(volume))/28)*10)/10;
    	    g_object_set(renderer->volume, "volume", avol, NULL);
        }
}

void audio_renderer_flush(audio_renderer_t *renderer) {
}

void audio_renderer_destroy(audio_renderer_t *renderer) {

    gst_app_src_end_of_stream (GST_APP_SRC(renderer->appsrc));
    gst_object_unref (renderer->appsrc);
    gst_element_set_state (renderer->pipeline, GST_STATE_NULL);
    gst_object_unref (renderer->pipeline);

    gst_object_unref (renderer->volume);
    if (renderer) {
        free(renderer);
    }
}
