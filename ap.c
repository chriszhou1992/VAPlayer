#include "ap.h"

/* This function is called when an error message is posted on the bus */
static void error(GstBus *bus, GstMessage *msg, apData *data) {
	GError *err;
	gchar *debug_info;

	GstCaps *caps;

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
static void eos(GstBus *bus, GstMessage *msg, apData *data) {
	g_print ("End-Of-Stream reached.\n");
	gtk_button_set_label(GTK_BUTTON(data->apButton), "Play Audio");
	gst_element_set_state (data->pipeline, GST_STATE_READY);
}

int init_ap(int argc, char *argv[], apData *data) {
	GstBus *bus;
	GstStateChangeReturn ret;

	char *formats[2] = {"alaw", "mulaw"};
	char *decoders[2] = {"alawdec", "mulawdec"};
	char *parsers[2] = {"audio/x-alaw", "audio/x-mulaw"};
	int i = -1;	
	if (argv != NULL) {
		if (strstr(argv[0], formats[0]) != NULL) {
			i = 0;
		} else if (strstr(argv[0], formats[1]) != NULL) {
			i = 1;
		} else {

			g_print("Sorry, the file format is not supported. We only support alaw and mulaw now.\n");
			return -1;
		}
	}
	/* Initialize GStreamer */
	gst_init (&argc, &argv);
	
	data->fileSrc = gst_element_factory_make("filesrc", "fileSrc");
	data->caps = gst_element_factory_make("capsfilter", "parser");
	data->decoder = gst_element_factory_make(decoders[i], "decoder");
	data->audioSink = gst_element_factory_make("alsasink", "audioSink");

	/* Create the empty pipeline */
	data->pipeline = gst_pipeline_new("pipeline");

	if (!data->pipeline || !data->fileSrc || !data->caps || !data->decoder || !data->audioSink) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	GstCaps *caps = gst_caps_new_simple (parsers[i],
									  "rate", G_TYPE_INT, 22050,
										"channels", G_TYPE_INT, 2,
									  NULL);
	g_object_set(data->fileSrc, "location", argv[0], NULL);
	g_object_set(data->caps, "caps", caps, NULL);
	free(argv[0]);

	/* Build the pipeline */
	gst_bin_add_many(GST_BIN(data->pipeline), data->fileSrc, data->caps, data->decoder, data->audioSink, NULL);
	
	if (!gst_element_link_many(data->fileSrc, data->caps, data->decoder, data->audioSink, NULL)) {
		g_printerr("Elements could not be linked.\n");
		gst_object_unref(data->pipeline);
    	return -1;
	}

	/* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
	bus = gst_element_get_bus(data->pipeline);
	gst_bus_add_signal_watch(bus);
	g_signal_connect (G_OBJECT(bus), "message::error", (GCallback)error, data);
	g_signal_connect (G_OBJECT(bus), "message::eos", (GCallback)eos, data);
	gst_object_unref(bus);

	/* Start playing */
	ret = gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_element_set_state (data->pipeline, GST_STATE_NULL);
		gst_object_unref (data->pipeline);
		return -1;
	}
	
	return 0;
}

void cleanup_ap(apData *data) {
	/* Free resources */
	if (data->pipeline != NULL && GST_IS_ELEMENT(data->pipeline)) {
		gst_element_set_state(data->pipeline, GST_STATE_NULL);
		gst_object_unref(data->pipeline);
	}
}
