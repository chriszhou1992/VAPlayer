#include "vp.h"

/* This function is called when an error message is posted on the bus */
static void error(GstBus *bus, GstMessage *msg, pData *data) {
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
static void eos(GstBus *bus, GstMessage *msg, pData *data) {
	data->rate = 1.0;
	g_print ("End-Of-Stream reached.\n");
	gst_element_set_state (data->pipeline, GST_STATE_READY);
}

static GstBusSyncReply create_window_vp(GstBus *bus, GstMessage *message, pData *data) {
	
	// ignore anything but 'prepare-xwindow-id' element messages
	if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
		return GST_BUS_PASS;

	if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
		return GST_BUS_PASS;
	g_print("%d\n", (int)data->recordWindowXID);
	if(data->recordWindowXID != 0) {
		GstXOverlay *xoverlay = GST_X_OVERLAY (GST_MESSAGE_SRC (message));
		gst_x_overlay_set_window_handle(xoverlay, data->recordWindowXID);
	} else {
   		g_warning ("Should have obtained video_window_xid by now!");
 	}

 	gst_message_unref (message);

 	return GST_BUS_DROP;
}


int init_vp(int argc, char *argv[], pData *data) {
	GstBus *bus;
	GstStateChangeReturn ret;
	char *formats[2] = {"mpeg4", "mjpeg"};
	char *encoders[2] = {"ffdec_mpeg4", "jpegdec"};
	char *parsers[2] = {"mpeg4videoparse", "jpegparse"};
	int i = -1;	
	if (argv != NULL) {
		if (strstr(argv[0], formats[0]) != NULL) {
			i = 0;
		} else if (strstr(argv[0], formats[1]) != NULL) {
			i = 1;
		} else {

			g_print("Sorry, the file format is not supported. We only support mpeg4 and mjpeg now.\n");
			return;
		}
	}
	cleanup_vp(data);
	/* Initialize GStreamer */
	gst_init (&argc, &argv);
	
	data->fileSrc = gst_element_factory_make("filesrc", "fileSrc");
	data->parser = gst_element_factory_make(parsers[i], "parser");
	data->decoder = gst_element_factory_make(encoders[i], "decoder");
	data->videoSink = gst_element_factory_make("autovideosink", "videoSink");

	/* Create the empty pipeline */
	data->pipeline = gst_pipeline_new("pipeline");

	if (!data->pipeline || !data->fileSrc || !data->parser || !data->decoder || !data->videoSink) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	g_object_set(data->fileSrc, "location", argv[0], NULL);
	free(argv[0]);

	/* Build the pipeline */
	gst_bin_add_many(GST_BIN(data->pipeline), data->fileSrc, data->parser, data->decoder, data->videoSink, NULL);
	
	if (!gst_element_link_many(data->fileSrc, data->parser, data->decoder, data->videoSink, NULL)) {
		g_printerr("Elements could not be linked.\n");
		gst_object_unref(data->pipeline);
    	return -1;
	}

	/* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
	bus = gst_element_get_bus(data->pipeline);
	gst_bus_add_signal_watch(bus);
	g_signal_connect (G_OBJECT(bus), "message::error", (GCallback)error, data);
	g_signal_connect (G_OBJECT(bus), "message::eos", (GCallback)eos, data);
	gst_bus_set_sync_handler(bus, (GstBusSyncHandler)create_window_vp, data);
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

void cleanup_vp(pData *data) {
	/* Free resources */
	if (data->pipeline != NULL && GST_IS_ELEMENT(data->pipeline)) {
		gst_element_set_state(data->pipeline, GST_STATE_NULL);
		gst_object_unref(data->pipeline);
	}
}
