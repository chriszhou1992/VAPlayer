#ifndef vr_H_
#define vr_H_

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdbool.h>

typedef struct _vrData {
	GstElement *pipeline;
	GstElement *recordSrc, *recordFilter, *recordSink;
	GstElement *recordEncoder, *recordMuxer, *fileSink;

	GstElement *tee, *playQueue, *saveQueue;
	GstPadTemplate *teeSrcPadTemplate;
  	GstPad *teeSavePad, *teePlayPad, *teeMonitorPad;
  	GstPad *saveQueuePad, *playQueuePad;

	gulong recordWindowXID;

	GtkWidget *recordButton, *leftButton, *rightButton, *upButton, *downButton, *resetPanButton, *resetTiltButton;
} vrData;

int init_vr(int argc, char *argv[], vrData *data);
void cleanup_vr(vrData *data);

void vr_get_file_name(gchar *format, gchar *filename);
void vr_print_and_free(char *argv[], char *filename);

static GstBusSyncReply vr_create_window(GstBus * bus, GstMessage * message, vrData *data);
static void vr_error(GstBus *bus, GstMessage *msg, vrData *data);
static void vr_eos(GstBus *bus, GstMessage *msg, vrData *data);

#endif
