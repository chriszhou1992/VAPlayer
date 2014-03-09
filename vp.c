#include "vp.h"

int init_vp(int argc, char *argv[], vpData *data) {
	GstBus *bus;
	GstStateChangeReturn ret;

	char *formats[2] = {"mpeg4", "mjpeg"};
	char *decoders[2] = {"ffdec_mpeg4", "jpegdec"};
	int fileType = vp_get_file_type(argv, formats);

	data->playbackSpeed = 1.0;

	if (fileType < 0) {
		g_print("Sorry, the file format is not supported. We only support mpeg4 and mjpeg now.\n");
		return -1;
	}

	/* Initialize GStreamer */
	gst_init (0, NULL);
	
	/* Create the elements */
	data->fileSrc = gst_element_factory_make("filesrc", "fileSrc");
	data->demuxer = gst_element_factory_make("avidemux", "demuxer");
	data->decoder = gst_element_factory_make(decoders[fileType], "decoder");
	data->videoSink = gst_element_factory_make("autovideosink", "videoSink");

	/* Create the empty pipeline */
	data->pipeline = gst_pipeline_new("pipeline");

	if (!data->pipeline || !data->fileSrc || !data->demuxer || !data->decoder || !data->videoSink) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	/* Set properties */
	g_object_set(data->fileSrc, "location", argv[0], NULL);

	/* Build the pipeline */
	gst_bin_add_many(GST_BIN(data->pipeline), data->fileSrc, data->demuxer, 
					 data->decoder, data->videoSink, NULL);
	
	if (!gst_element_link_many(data->fileSrc, data->demuxer, NULL) ||
		!gst_element_link_many(data->decoder, data->videoSink, NULL)) {
		g_printerr("Elements could not be linked.\n");
		gst_object_unref(data->pipeline);
    	return -1;
	}

	/* Connect to the pad-added signal */
  	g_signal_connect (data->demuxer, "pad-added", G_CALLBACK(vp_demuxer_pad_added_handler), data);

	/* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
	bus = gst_element_get_bus(data->pipeline);
	gst_bus_add_signal_watch(bus);
	g_signal_connect (G_OBJECT(bus), "message::error", (GCallback)vp_error, data);
	g_signal_connect (G_OBJECT(bus), "message::eos", (GCallback)vp_eos, data);
	gst_bus_set_sync_handler(bus, (GstBusSyncHandler)vp_create_window, data);
	gst_object_unref(bus);

	/* Start playing */
	ret = gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_element_set_state(data->pipeline, GST_STATE_NULL);
		gst_object_unref(data->pipeline);
		return -1;
	}
	
	g_print("Now playing %s\n", argv[0]);
	free(argv[0]);

	return 0;
}

void cleanup_vp(vpData *data) {
	if (data->pipeline != NULL && GST_IS_ELEMENT(data->pipeline)) {
		gst_element_set_state(data->pipeline, GST_STATE_NULL);
		gst_object_unref(data->pipeline);
	}
}

int vp_get_file_type(char *argv[], char *formats[]) {
	int i = -1;
	if (strstr(argv[0], formats[0]) != NULL) {
		i = 0;
	} else if (strstr(argv[0], formats[1]) != NULL) {
		i = 1;
	}
	return i;
}

static void vp_demuxer_pad_added_handler(GstElement *src, GstPad *new_pad, vpData *data) {
	GstPad *sink_pad = gst_element_get_static_pad (data->decoder, "sink");
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;
   
	g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));
   
	/* If our decoder is already linked, we have nothing to do here */
	if (gst_pad_is_linked (sink_pad)) {
		g_print ("We are already linked. Ignoring.\n");
		goto exit;
	}
   
	/* Check the new pad's type */
	new_pad_caps = gst_pad_get_caps (new_pad);
	new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
	new_pad_type = gst_structure_get_name (new_pad_struct);
	if (!g_str_has_prefix (new_pad_type, "image/") && !g_str_has_prefix (new_pad_type, "video/")) {
		g_print ("It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
		goto exit;
	}
   
	/* Attempt the link */
	ret = gst_pad_link (new_pad, sink_pad);
	if (GST_PAD_LINK_FAILED (ret)) {
		g_print ("Type is '%s' but link failed.\n", new_pad_type);
	} else {
		g_print ("Link succeeded (type '%s').\n", new_pad_type);
	}
   
	exit:
		/* Unreference the new pad's caps, if we got them */
		if (new_pad_caps != NULL)
			gst_caps_unref (new_pad_caps);

		/* Unreference the sink pad */
		gst_object_unref (sink_pad);
}

/* Change output window */
static GstBusSyncReply vp_create_window(GstBus *bus, GstMessage *message, vpData *data) {
	/* Ignore anything but 'prepare-xwindow-id' element messages */
	if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
		return GST_BUS_PASS;

	if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
		return GST_BUS_PASS;

	if(data->recordWindowXID != 0) {
		GstXOverlay *xoverlay = GST_X_OVERLAY (GST_MESSAGE_SRC (message));
		gst_x_overlay_set_window_handle(xoverlay, data->recordWindowXID);
	} else {
   		g_warning ("Should have obtained video_window_xid by now!");
 	}

 	gst_message_unref (message);

 	return GST_BUS_DROP;
}

/* This function is called when an error message is posted on the bus */
static void vp_error(GstBus *bus, GstMessage *msg, vpData *data) {
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
static void vp_eos(GstBus *bus, GstMessage *msg, vpData *data) {
	data->playbackSpeed = 1.0;
	data->playImage = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
	gtk_button_set_image((GtkButton *)data->playPauseButton, data->playImage);
	g_print("End-Of-Stream reached.\n");
	gst_element_set_state (data->pipeline, GST_STATE_READY);
}
