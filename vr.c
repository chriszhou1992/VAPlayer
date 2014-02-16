#include "vr.h"

/* This function is called when an error message is posted on the bus */
static void error(GstBus *bus, GstMessage *msg, Data *data) {
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
static void eos(GstBus *bus, GstMessage *msg, Data *data) {
	g_print ("End-Of-Stream reached.\n");
	gst_element_set_state (data->pipeline, GST_STATE_READY);
}

static GstBusSyncReply create_window(GstBus *bus, GstMessage *message, Data *data) {

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

int init_vr(int argc, char *argv[], Data *data) {
	GstBus *bus;
	GstStateChangeReturn ret;
	GstCaps *recordCaps;

	gint width;
	gint height;
	gint frame;
	char *format;

	/* Initialize GStreamer */
	gst_init (&argc, &argv);

	width = atoi(argv[0]);
	height = atoi(argv[1]);
	frame = atoi(argv[2]);
	format = argv[3];

	int i;
	for (i = 0; i < 3; i++) {
		free(argv[i]);
	}

	/* Create the elements */
	data->recordSrc = gst_element_factory_make("v4l2src", "recordSrc");
	data->recordFilter = gst_element_factory_make("capsfilter", "recordFilter");
	data->recordConverter = gst_element_factory_make("ffmpegcolorspace", "recordConverter");
	data->recordSink = gst_element_factory_make("autovideosink", "recordSink");
	data->recordEncoder = gst_element_factory_make(format, "recordEncoder");
	data->fileSink = gst_element_factory_make("filesink", "fileSink");

	g_print("Encoded with %s\n", format);

	data->tee = gst_element_factory_make("tee", "tee");
	data->playQueue = gst_element_factory_make("queue", "playQueue");
	data->saveQueue = gst_element_factory_make("queue", "saveQueue");
		
	/* Create the empty pipeline */
	data->pipeline = gst_pipeline_new("recordPipeline");

	if (!data->pipeline || !data->recordSrc || !data->recordFilter || !data->recordConverter || !data->recordSink ||
		!data->recordEncoder || !data->fileSink ||
		!data->tee || !data->playQueue || !data->saveQueue) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	/* Specify what kind of video is wanted from the camera */
	recordCaps = gst_caps_new_simple ("video/x-raw-yuv",
									  "width", G_TYPE_INT, width,
									  "height", G_TYPE_INT, height,
									  "framerate", GST_TYPE_FRACTION, frame, 1,
									  NULL);

	char filename[30];
	filename[0] = '\0';
	strcat(filename, "Video/");
	char filenum[20];	
	sprintf(filenum, "%d", rand());
	strcat(filename, filenum);
	if (strcmp(format, "ffenc_mpeg4") == 0) {
		strcat(filename, ".mpeg4");
	} else {
		strcat(filename, ".mjpeg");
	}
	free(format);
	g_print("%s\n", filename);
	/* Set properties */
	g_object_set(data->recordSrc, "device", "/dev/video1", NULL);
	g_object_set(data->recordFilter, "caps", recordCaps, NULL);
	g_object_set(data->fileSink, "location", filename, NULL);

	gst_caps_unref(recordCaps);

	/* Build the pipeline */
	gst_bin_add_many(GST_BIN(data->pipeline), 
					 data->recordSrc, data->recordFilter, data->recordConverter, data->recordSink, 
					 data->recordEncoder, data->fileSink, 
					 data->tee, data->playQueue, data->saveQueue, NULL);
	

	if (!gst_element_link_many(data->recordSrc, data->recordFilter, data->tee, NULL) ||
		!gst_element_link_many(data->saveQueue, data->recordEncoder, data->fileSink, NULL) ||
		!gst_element_link_many(data->playQueue, data->recordSink, NULL)) {
		g_printerr("Elements could not be linked.\n");
		gst_object_unref(data->pipeline);
    	return -1;
	}


	/* Manually link the Tee, which has "Request" pads */
	data->teeSrcPadTemplate = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(data->tee), "src%d");
	data->teeSavePad = gst_element_request_pad(data->tee, data->teeSrcPadTemplate, NULL, NULL);
	g_print ("Obtained request pad %s for save branch.\n", gst_pad_get_name(data->teeSavePad));
	data->saveQueuePad = gst_element_get_static_pad(data->saveQueue, "sink");
	data->teePlayPad = gst_element_request_pad(data->tee, data->teeSrcPadTemplate, NULL, NULL);
	g_print ("Obtained request pad %s for play branch.\n", gst_pad_get_name(data->teePlayPad));
	data->playQueuePad = gst_element_get_static_pad(data->playQueue, "sink");
	if (gst_pad_link(data->teeSavePad, data->saveQueuePad) != GST_PAD_LINK_OK ||
		gst_pad_link(data->teePlayPad, data->playQueuePad) != GST_PAD_LINK_OK) {
  		g_printerr ("Tee could not be linked.\n");
		gst_object_unref(data->pipeline);
		return -1;
	}
	gst_object_unref(data->saveQueuePad);
	gst_object_unref(data->playQueuePad);

	/* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
	bus = gst_element_get_bus(data->pipeline);
	gst_bus_add_signal_watch(bus);
	g_signal_connect (G_OBJECT(bus), "message::error", (GCallback)error, data);
	g_signal_connect (G_OBJECT(bus), "message::eos", (GCallback)eos, data);
	gst_bus_set_sync_handler(bus, (GstBusSyncHandler)create_window, data);
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

void cleanup_vr(Data *data) {
	/* Release the request pads from the Tee, and unref them */
	gst_element_release_request_pad(data->tee, data->teeSavePad);
	gst_element_release_request_pad(data->tee, data->teePlayPad);
	gst_object_unref(data->teeSavePad);
	gst_object_unref(data->teePlayPad);
	/* Free resources */
	gst_element_set_state(data->pipeline, GST_STATE_NULL);
	gst_object_unref(data->pipeline);
}
