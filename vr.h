#ifndef vr_H_
#define vr_H_

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

#include <string.h>

typedef struct _Data {
	GstElement *pipeline;
	GstElement *recordSrc, *recordFilter, *recordConverter, *recordSink;
	GstElement *recordEncoder, *fileSink;

	GstElement *tee, *playQueue, *saveQueue;
	GstPadTemplate *teeSrcPadTemplate;
  	GstPad *teeSavePad, *teePlayPad;
  	GstPad *saveQueuePad, *playQueuePad;

	gulong recordWindowXID;
} Data;

static void error(GstBus *bus, GstMessage *msg, Data *data);
static void eos(GstBus *bus, GstMessage *msg, Data *data);
static GstBusSyncReply create_window (GstBus * bus, GstMessage * message, Data *data);

int init_vr(int argc, char *argv[], Data *data);
void cleanup_vr(Data *data);

#endif
