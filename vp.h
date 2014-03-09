#ifndef vp_H_
#define vp_H_

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <gtk/gtk.h>
#include <string.h>

typedef struct _vpData {
	GstElement *pipeline;
	GstElement *fileSrc, *demuxer, *decoder, *videoSink;

	gulong recordWindowXID;

	gdouble playbackSpeed;
	GtkWidget *playImage;
	GtkWidget *pauseImage;
	GtkWidget *fileButton, *playPauseButton, *rewindButton, *forwardButton;
} vpData;


int init_vp(int argc, char *argv[], vpData *data);
void cleanup_vp(vpData *data);

int vp_get_file_type(char *argv[], char *formats[]);

static void vp_demuxer_pad_added_handler(GstElement *src, GstPad *new_pad, vpData *data);
static GstBusSyncReply vp_create_window(GstBus *bus, GstMessage *message, vpData *data);
static void vp_error(GstBus *bus, GstMessage *msg, vpData *data);
static void vp_eos(GstBus *bus, GstMessage *msg, vpData *data);

#endif
