#ifndef ap_H_
#define ap_H_

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <gtk/gtk.h>
#include <string.h>

typedef struct _apData {
	GstElement *pipeline;
	GstElement *fileSrc, *demuxer, *decoder, *audioSink;

	GtkWidget *apButton;
} apData;


int init_ap(int argc, char *argv[], apData *data);
void cleanup_ap(apData *data);

int get_file_type(char *argv[], char *formats[]);

static void demuxer_pad_added_handler (GstElement *src, GstPad *new_pad, apData *data);
static void ap_error(GstBus *bus, GstMessage *msg, apData *data);
static void ap_eos(GstBus *bus, GstMessage *msg, apData *data);

#endif
