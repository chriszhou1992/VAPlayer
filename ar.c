#include "ar.h"

int init_ar(int argc, char *argv[], arData *data) {
	GstBus *bus;
	GstStateChangeReturn ret;
	GstCaps *recordCaps;

	gchar filename[30];

	/* Specifications */
	gint samplingRate = atoi(argv[0]);
	gint samplingSize = atoi(argv[1]);
	gint numChannels = atoi(argv[2]);
	gchar *format = argv[3];
	gchar *device = "hw:1";
	bool raw = (strcmp(format, "none") == 0);

	/* Initialize GStreamer */
	gst_init(0, NULL);

	
	/* Create the elements */
	data->recordSrc = gst_element_factory_make("alsasrc", "recordSrc");
	data->recordSampler = gst_element_factory_make("audioresample", "recordSampler");
	data->recordFilter1 = gst_element_factory_make("capsfilter", "recordFilter1");
	data->recordConverter = gst_element_factory_make("audioconvert", "recordConverter");
	data->recordFilter2 = gst_element_factory_make("capsfilter", "recordFilter2");
	if (!raw) {
		data->recordEncoder = gst_element_factory_make(format, "recordEncoder");
		data->recordMuxer = gst_element_factory_make("avimux", "recordMuxer");
	}
	data->fileSink = gst_element_factory_make("filesink", "fileSink");

	/* Create the empty pipeline */
	data->pipeline = gst_pipeline_new("recordPipeline");
	
	if (!raw) {
		if (!data->pipeline || !data->recordSrc || !data->recordSampler || !data->recordFilter1 || 
			!data->recordConverter || !data->recordFilter2 || 
			!data->recordEncoder || !data->recordMuxer || !data->fileSink) {
			g_printerr("Not all elements could be created.\n");
			return -1;
		}
	} else {
		format = "raw";
		if (!data->pipeline || !data->recordSrc || !data->recordSampler || !data->recordFilter1 || 
			!data->recordConverter || !data->recordFilter2 || !data->fileSink) {
			g_printerr("Not all elements could be created.\n");
			return -1;
		}
	}

	printf("%s\n", format);

	/* Specify what kind of audio is wanted from the microphone */
	recordCaps = gst_caps_new_simple ("audio/x-raw-int",
									  "rate", G_TYPE_INT, samplingRate,
									  NULL);
	g_object_set(data->recordFilter1, "caps", recordCaps, NULL);
	gst_caps_unref(recordCaps);
	recordCaps = gst_caps_new_simple ("audio/x-raw-int",
									  "channels", G_TYPE_INT, numChannels,
									  "width", G_TYPE_INT, samplingSize,
									  "depth", G_TYPE_INT, samplingSize,
									  "signed", G_TYPE_BOOLEAN, TRUE,
									  NULL);
	g_object_set(data->recordFilter2, "caps", recordCaps, NULL);
	gst_caps_unref(recordCaps);

	/* Generate a name for the file to be stored */
	get_file_name(format, filename);

	/* Set properties */
	g_object_set(data->recordSrc, "device", device, NULL);
	g_object_set(data->fileSink, "location", filename, NULL);

	/* Build the pipeline */
	if (!raw) {
		gst_bin_add_many(GST_BIN(data->pipeline), 
						 data->recordSrc, data->recordSampler, data->recordFilter1, 
						 data->recordConverter, data->recordFilter2,
						 data->recordEncoder, data->recordMuxer, data->fileSink, NULL);

		if (!gst_element_link_many(data->recordSrc, data->recordSampler, data->recordFilter1, 
								   data->recordConverter, data->recordFilter2, 
								   data->recordEncoder, data->recordMuxer, data->fileSink, NULL)) {
			g_printerr("Elements could not be linked.\n");
			gst_object_unref(data->pipeline);
			return -1;
		}
	} else {
		gst_bin_add_many(GST_BIN(data->pipeline), 
						 data->recordSrc, data->recordSampler, data->recordFilter1, 
						 data->recordConverter, data->recordFilter2, data->fileSink, NULL);

		if (!gst_element_link_many(data->recordSrc, data->recordSampler, data->recordFilter1, 
								   data->recordConverter, data->recordFilter2, data->fileSink, NULL)) {
			g_printerr("Elements could not be linked.\n");
			gst_object_unref(data->pipeline);
			return -1;
		}
	

	}

	/* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
	bus = gst_element_get_bus(data->pipeline);
	gst_bus_add_signal_watch(bus);
	g_signal_connect (G_OBJECT(bus), "message::error", (GCallback)ar_error, data);
	g_signal_connect (G_OBJECT(bus), "message::eos", (GCallback)ar_eos, data);
	gst_object_unref(bus);

	/* Start playing */
	ret = gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_element_set_state (data->pipeline, GST_STATE_NULL);
		gst_object_unref (data->pipeline);
		return -1;
	}

	print_and_free(argv, filename);
	
	return 0;
}

void cleanup_ar(arData *data) {
	gst_element_set_state(data->pipeline, GST_STATE_NULL);
	gst_object_unref(data->pipeline);
}

void get_file_name(gchar *format, gchar *filename) {
	filename[0] = '\0';
	strcat(filename, "Audio/");
	gchar filenum[20];	
	sprintf(filenum, "%d", rand());
	strcat(filename, filenum);
	if (strcmp(format, "mulawenc") == 0) {
		strcat(filename, ".mulaw");
	} else if (strcmp(format, "alawenc") == 0){
		strcat(filename, ".alaw");
	} else {
		strcat(filename, ".raw");
	}
}

void print_and_free(char *argv[], char *filename) {
	int i;
	
	g_print("Encoded with %s\n", argv[3]);
	g_print("Sample Rate: %s\n", argv[0]);
	g_print("Sample Size: %s\n", argv[1]);
	g_print("Channels: %s\n", argv[2]);
	g_print("File: %s\n", filename);

	/*for (i = 0; i < 4; i++) {
		free(argv[i]);
	}*/
}

/* This function is called when an error message is posted on the bus */
static void ar_error(GstBus *bus, GstMessage *msg, arData *data) {
	GError *err;
	gchar *debug_info;

	/* Print error details on the screen */
	gst_message_parse_error (msg, &err, &debug_info);
	g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
	g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
	g_clear_error (&err);
	g_free (debug_info);

	/* Set the pipeline to READY (which stops playback) */
	gst_element_set_state (data->pipeline, GST_STATE_READY);
}
 
/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void ar_eos(GstBus *bus, GstMessage *msg, arData *data) {
	g_print ("End-Of-Stream reached.\n");
	gst_element_set_state (data->pipeline, GST_STATE_READY);
}

