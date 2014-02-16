#ifndef ap_H_
#define ap_H_

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <gtk/gtk.h>
#include <string.h>

typedef struct _apData {
	GstElement *pipeline;
	GstElement *fileSrc, *caps, *decoder, *audioSink;

	GtkWidget *apButton;
} apData;


int init_ap(int argc, char *argv[], apData *data);
void cleanup_ap(apData *data);

#endif
