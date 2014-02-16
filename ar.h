#ifndef ar_H_
#define ar_H_

#include <gst/gst.h>

#include <string.h>

typedef struct _aData {
	GstElement *pipeline;
	GstElement *recordSrc, *recordSampler, *recordFilter1, *recordConverter, *recordFilter2, *recordSink;
	GstElement *recordEncoder, *fileSink;

	GstElement *tee, *playQueue, *saveQueue;
	GstPadTemplate *teeSrcPadTemplate;
  	GstPad *teeSavePad, *teePlayPad;
  	GstPad *saveQueuePad, *playQueuePad;

	gulong recordWindowXID;
} aData;

static void a_error(GstBus *bus, GstMessage *msg, aData *data);
static void a_eos(GstBus *bus, GstMessage *msg, aData *data);

int init_ar(int argc, char *argv[], aData *data);
void cleanup_ar(aData *data);

#endif
