#ifndef vp_H_
#define vp_H_

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <gtk/gtk.h>
#include <string.h>

typedef struct _pData {
	GstElement *pipeline;
	GstElement *fileSrc, *parser, *decoder, *videoSink;

	gulong recordWindowXID;

  	GstState state;                 /* Current state of the pipeline */
	gdouble rate;      /* Current playback rate */
} pData;

static GstBusSyncReply create_window_vp (GstBus * bus, GstMessage * message, pData *data);

int init_vp(int argc, char *argv[], pData *data);
void cleanup_vp(pData *data);

#endif
