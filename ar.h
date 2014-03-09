#ifndef ar_H_
#define ar_H_

#include <gst/gst.h>
#include <string.h>
#include <stdbool.h>

typedef struct _arData {
	GstElement *pipeline;
	GstElement *recordSrc, *recordSampler, *recordFilter1, *recordConverter, *recordFilter2;
	GstElement *recordEncoder, *recordMuxer, *fileSink;
} arData;

int init_ar(int argc, char *argv[], arData *data);
void cleanup_ar(arData *data);

void get_file_name(gchar *format, gchar *filename);
void print_and_free(char *argv[], char *filename);

static void ar_error(GstBus *bus, GstMessage *msg, arData *data);
static void ar_eos(GstBus *bus, GstMessage *msg, arData *data);

#endif
