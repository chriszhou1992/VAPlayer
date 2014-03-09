#include "gui.h"
#include "cc.c"

static gboolean delete_event_cb(GtkWidget *widget, GdkEvent *event) {
	/* If you return FALSE in the "delete-event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */
    return FALSE;
}

static void destroy_cb(GtkWidget *widget) {
    gtk_main_quit ();
}

void setup_main_window(Components *components) {
	/* create a new window */
    components->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* Handle "delete-event" signal */
    g_signal_connect(components->window, "delete-event", G_CALLBACK(delete_event_cb), NULL);
	/* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete-event" callback. */
    g_signal_connect(components->window, "destroy", G_CALLBACK(destroy_cb), NULL);

	gtk_window_set_title(GTK_WINDOW(components->window), "CS414 MP1");
	gtk_window_set_default_size(GTK_WINDOW(components->window), 300, 0);
}

char *videoArgv[4];

static void record_video_cb(GtkWidget *widget, Components *components) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widget)), "Stop Video Recording") == 0) {
		cleanup_vr(&components->vrD);
		gtk_button_set_label(GTK_BUTTON(components->vrButton), "Record Video");
		gtk_widget_hide(components->recorder);
		
		return;
	}
	
	gtk_widget_show_all(components->vrOptions);
}

void setup_vr_button(Components *components) {
	/* Creates a new button */
    components->vrButton = gtk_button_new_with_label ("Record Video");
	/* Handle"clicked" signal */
    g_signal_connect(components->vrButton, "clicked", G_CALLBACK(record_video_cb), components);
}

GtkWidget *buttons[7];
char *vrOptions[] = { "320|240|30|ffenc_mpeg4|", "320|240|30|jpegenc|", 
					"640|480|30|ffenc_mpeg4|", "640|480|30|jpegenc|", 
					"960|720|15|ffenc_mpeg4|", "960|720|15|jpegenc|",
					"Record with the following resolution and frame rate!" };

static void vr_options_cb(GtkWidget *widget, Components *components) {
	
	gtk_widget_hide(components->vrOptions);
	
	int i;
	char *option;
	for (i = 0; i < (int)(sizeof(buttons)/sizeof(buttons[0])); i++) {
		if (gtk_toggle_button_get_active((GtkToggleButton *)buttons[i])) {
			option = vrOptions[i];
		}
	}

	gtk_button_set_label(GTK_BUTTON(components->vrButton), "Stop Video Recording");

	if (!strcmp(option, "Record with the following resolution and frame rate!") == 0) {
		char *spec = option;
		char *temp = option;
		char *argv[4];
		i = 0;
		int count = 0;
		while (*temp != '\0') {
			if(*temp == '|') {
				argv[i] = malloc(count+1);
				strncpy(argv[i], spec, count);
				*(argv[i]+count) = '\0';
				temp++;		
				spec = temp;
				i++;
				count = 0;
			} else {
				temp++;
				count++;
			}
		}
	
		gint width = atoi(argv[0]);
		gint height = atoi(argv[1]);

		gtk_window_resize(GTK_WINDOW(components->recorder), width, height);
		gtk_widget_show_all(components->recorder);

		if(init_vr(4, argv, &components->vrD) < 0) {
			gtk_button_set_label(GTK_BUTTON(components->vrButton), "Record Video");
		}
	} else {
		char *spec = videoArgv[0];
		if (*spec == '3') {
			videoArgv[0] = malloc(20);
			strcpy(videoArgv[0], "320");
			videoArgv[1] = malloc(20);
			strcpy(videoArgv[1], "240");
		} else if (*spec == '6') {
			videoArgv[0] = malloc(20);
			strcpy(videoArgv[0], "640");
			videoArgv[1] = malloc(20);
			strcpy(videoArgv[1], "480");
		} else {
			videoArgv[0] = malloc(20);
			strcpy(videoArgv[0], "960");
			videoArgv[1] = malloc(20);
			strcpy(videoArgv[1], "720");
		}
		free(spec);
		printf("%s x %s\n", videoArgv[0], videoArgv[1]);
		gint width = atoi(videoArgv[0]);
		gint height = atoi(videoArgv[1]);
		
		gtk_window_resize(GTK_WINDOW(components->recorder), width, height);
		gtk_widget_show_all(components->recorder);
		if(init_vr(4, videoArgv, &components->vrD) < 0) {
			gtk_button_set_label(GTK_BUTTON(components->vrButton), "Record Video");
		}
	}
}

static void destroy_vr_options_cb(GtkWidget *widget) {
	gtk_widget_hide(widget);
}

int dev;

void pan_left_cb(GtkWidget *widget, Components *components) {
	pan_tilt(dev, 128, 0, 0);
}

void pan_right_cb(GtkWidget *widget, Components *components) {
	pan_tilt(dev, -128, 0, 0);
}

void tilt_up_cb(GtkWidget *widget, Components *components) {
	pan_tilt(dev, 0, -128, 0);
}

void tilt_down_cb(GtkWidget *widget, Components *components) {
	pan_tilt(dev, 0, 128, 0);
}

void reset_pan_cb(GtkWidget *widget, Components *components) {
	pan_tilt(dev, 0, 0, 1);
}

void reset_tilt_cb(GtkWidget *widget, Components *components) {
	pan_tilt(dev, 0, 0, 2);
}

/* This function gets called when currently selected item changes */
static void resolution_cb(GtkComboBox *combo) {
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text(combo);
	g_free(videoArgv[0]);
    videoArgv[0] = string;
}

/* This function gets called when currently selected item changes */
static void framerate_cb(GtkComboBox *combo) {
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text(combo);
	g_free(videoArgv[2]);
    videoArgv[2] = string;
}

void setup_vr_options(Components *components) {
	GSList *group;
	GtkWidget *button, *controls, *actionArea;
	gint padding = 5;
	components->vrOptions = gtk_dialog_new_with_buttons("Video Options", GTK_WINDOW(components->window), GTK_DIALOG_MODAL, NULL);
	/* Center the menu of options */
	gtk_window_set_position(GTK_WINDOW(components->vrOptions), GTK_WIN_POS_CENTER);

    buttons[0] = gtk_radio_button_new_with_label(NULL, "Resolution: 320 x 240 | Frame Rate: 30 | Compression Format: mpeg4");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), buttons[0], TRUE, TRUE, padding);

	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(buttons[0]));
    buttons[1] = gtk_radio_button_new_with_label(group, "Resolution: 320 x 240 | Frame Rate: 30 | Compression Format: mjpeg");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), buttons[1], TRUE, TRUE, padding);

	buttons[2] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[1]), "Resolution: 640 x 480 | Frame Rate: 30 | Compression Format: mpeg4");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[2]), TRUE);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), buttons[2], TRUE, TRUE, padding);
	
    buttons[3] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[2]), "Resolution: 640 x 480 | Frame Rate: 30 | Compression Format: mjpeg");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), buttons[3], TRUE, TRUE, padding);

	buttons[4] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[3]), "Resolution: 960 x 720 | Frame Rate: 15 | Compression Format: mpeg4");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), buttons[4], TRUE, TRUE, padding);

	buttons[5] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[4]), "Resolution: 960 x 720 | Frame Rate: 15 | Compression Format: mjpeg");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), buttons[5], TRUE, TRUE, padding);

	buttons[6] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[5]), "Record with the following resolution and frame rate!");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), buttons[6], TRUE, TRUE, padding);


	char *resolutions[3] = {"320 x 240", "640 x 480", "960 x 720"};
	char *framerates[12] = {"5", "10", "15", "20", "25", "30"};
	GtkWidget *frame;
	GtkWidget *combo;
	int i;
	controls = gtk_hbox_new(FALSE, 0);

	videoArgv[0] = malloc(20);
	strcpy(videoArgv[0], "640 x 480");
	videoArgv[2] = malloc(20);
	strcpy(videoArgv[2], "30");
	videoArgv[3] = malloc(20);
	strcpy(videoArgv[3], "raw");

    /* Create frame */
    frame = gtk_frame_new("Resolution");
    gtk_box_pack_start(GTK_BOX(controls), frame, TRUE, TRUE, 10);
    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add(GTK_CONTAINER(frame), combo);
	for (i = 0; i < 3; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX(combo), resolutions[i]);
		if (i == 1) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
    /* Connect signal to tour handler function */
    g_signal_connect( G_OBJECT( combo ), "changed", G_CALLBACK(resolution_cb), NULL);

	/* Create frame */
    frame = gtk_frame_new("Frame Rate");
    gtk_box_pack_start(GTK_BOX(controls), frame, TRUE, TRUE, 10);
    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add(GTK_CONTAINER(frame), combo);
	for (i = 0; i < 6; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX(combo), framerates[i]);
		if (i == 5) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
    /* Connect signal to tour handler function */
    g_signal_connect( G_OBJECT( combo ), "changed", G_CALLBACK(framerate_cb), NULL);
    
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), controls, TRUE, TRUE, padding);


    button = gtk_button_new_with_label("Start Video Recording");
    gtk_box_pack_start(GTK_BOX (GTK_DIALOG(components->vrOptions)->vbox), button, TRUE, TRUE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(vr_options_cb), components);

	gtk_window_set_default_size(GTK_WINDOW(components->vrOptions), 320, 240);
	g_signal_connect(components->vrOptions, "delete-event", G_CALLBACK(destroy_vr_options_cb), NULL);

	components->vrD.leftButton = gtk_button_new_with_label("Pan Left");
  	g_signal_connect(G_OBJECT(components->vrD.leftButton), "clicked", G_CALLBACK(pan_left_cb), components);

	components->vrD.rightButton = gtk_button_new_with_label("Pan Right");
  	g_signal_connect(G_OBJECT(components->vrD.rightButton), "clicked", G_CALLBACK(pan_right_cb), components);
	
  	components->vrD.upButton = gtk_button_new_with_label("Tilt Up");
  	g_signal_connect(G_OBJECT(components->vrD.upButton), "clicked", G_CALLBACK (tilt_up_cb), components);
	
  	components->vrD.downButton = gtk_button_new_with_label("Tilt Down");
  	g_signal_connect(G_OBJECT(components->vrD.downButton), "clicked", G_CALLBACK(tilt_down_cb), components);

	components->vrD.resetPanButton = gtk_button_new_with_label("Reset Pan");
  	g_signal_connect(G_OBJECT(components->vrD.resetPanButton), "clicked", G_CALLBACK(reset_pan_cb), components);

	components->vrD.resetTiltButton = gtk_button_new_with_label("Reset Tilt");
  	g_signal_connect(G_OBJECT(components->vrD.resetTiltButton), "clicked", G_CALLBACK(reset_tilt_cb), components);

	controls = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(controls), components->vrD.leftButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.resetPanButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(controls), components->vrD.rightButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.upButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.resetTiltButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.downButton, TRUE, TRUE, 2);
	
	actionArea = gtk_dialog_get_action_area(GTK_DIALOG(components->vrOptions));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->vrOptions)->vbox), controls, FALSE, FALSE, 3);
}

/* This function is called when the GUI toolkit creates the physical window that will hold the video. */
static void vr_realize_cb(GtkWidget *widget, Components *components) {
	GdkWindow *window = gtk_widget_get_window(widget);

	if (!gdk_window_ensure_native(window))
		g_error ("Couldn't create native window needed for GstXOverlay!");

	/* Retrieve window handler from GDK */
	#if defined (GDK_WINDOWING_WIN32)
		components->vrD.recordWindowXID = (guintptr)GDK_WINDOW_HWND(window);
	#elif defined (GDK_WINDOWING_QUARTZ)
		components->vrD.recordWindowXID = gdk_quartz_window_get_nsview(window);
	#elif defined (GDK_WINDOWING_X11)
		components->vrD.recordWindowXID = GDK_WINDOW_XID(window);
	#endif
}

static void delete_recorder_cb(GtkWidget *widget, GdkEvent *event, Components *components) {
	cleanup_vr(&components->vrD);
	videoArgv[0] = malloc(20);
	
	gtk_widget_hide(widget);
	gtk_button_set_label(GTK_BUTTON(components->vrButton), "Record Video");
}

void setup_vr_window(Components *components) {
	GtkWidget *controls;
	GtkWidget *actionArea;

	dev = open("/dev/video1", 0);

	components->recorder = gtk_dialog_new_with_buttons("CS414 Recorder", GTK_WINDOW(components->window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	gtk_window_set_transient_for(GTK_WINDOW(components->recorder),GTK_WINDOW(components->window));
	/* Center the recorder */
	gtk_window_set_position(GTK_WINDOW(components->recorder), GTK_WIN_POS_CENTER);

	components->vrWindow = gtk_drawing_area_new();
	g_signal_connect(components->vrWindow, "realize", G_CALLBACK(vr_realize_cb), components);	

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->recorder)->vbox), components->vrWindow, TRUE, TRUE, 0);

	components->vrD.leftButton = gtk_button_new_with_label("Pan Left");
  	g_signal_connect(G_OBJECT(components->vrD.leftButton), "clicked", G_CALLBACK(pan_left_cb), components);

	components->vrD.rightButton = gtk_button_new_with_label("Pan Right");
  	g_signal_connect(G_OBJECT(components->vrD.rightButton), "clicked", G_CALLBACK(pan_right_cb), components);
	
  	components->vrD.upButton = gtk_button_new_with_label("Tilt Up");
  	g_signal_connect(G_OBJECT(components->vrD.upButton), "clicked", G_CALLBACK (tilt_up_cb), components);
	
  	components->vrD.downButton = gtk_button_new_with_label("Tilt Down");
  	g_signal_connect(G_OBJECT(components->vrD.downButton), "clicked", G_CALLBACK(tilt_down_cb), components);

	components->vrD.resetPanButton = gtk_button_new_with_label("Reset Pan");
  	g_signal_connect(G_OBJECT(components->vrD.resetPanButton), "clicked", G_CALLBACK(reset_pan_cb), components);

	components->vrD.resetTiltButton = gtk_button_new_with_label("Reset Tilt");
  	g_signal_connect(G_OBJECT(components->vrD.resetTiltButton), "clicked", G_CALLBACK(reset_tilt_cb), components);

	controls = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(controls), components->vrD.leftButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.resetPanButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(controls), components->vrD.rightButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.upButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.resetTiltButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX (controls), components->vrD.downButton, TRUE, TRUE, 2);
	
	actionArea = gtk_dialog_get_action_area(GTK_DIALOG(components->recorder));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->recorder)->vbox), controls, FALSE, FALSE, 3);

	g_signal_connect(components->recorder, "delete-event", G_CALLBACK(delete_recorder_cb), components);
}

static void play_video_cb(GtkWidget *widget, Components *components) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widget)), "Close Video Player") == 0) {
		cleanup_vp(&components->vpD);
		gtk_widget_hide(components->player);
		gtk_button_set_label(GTK_BUTTON(components->vpButton), "Play Video");
		return;
	}
	
	gtk_button_set_label(GTK_BUTTON(widget), "Close Video Player");
	gtk_widget_show_all(components->player);
}

void setup_vp_button(Components *components) {
	/* Creates a new button */
    components->vpButton = gtk_button_new_with_label("Play Video");
	/* Handle"clicked" signal */
    g_signal_connect(components->vpButton, "clicked", G_CALLBACK(play_video_cb), components);
	/* Signal that no video is being played */
	components->vpD.pipeline = NULL;
}

/* This function is called when the GUI toolkit creates the physical window that will hold the video. */
static void vp_realize_cb(GtkWidget *widget, Components *components) {
	GdkWindow *window = gtk_widget_get_window(widget);

	if (!gdk_window_ensure_native(window))
		g_error ("Couldn't create native window needed for GstXOverlay!");

	/* Retrieve window handler from GDK */
	#if defined (GDK_WINDOWING_WIN32)
		components->vpD.recordWindowXID = (guintptr)GDK_WINDOW_HWND(window);
	#elif defined (GDK_WINDOWING_QUARTZ)
		components->vpD.recordWindowXID = gdk_quartz_window_get_nsview(window);
	#elif defined (GDK_WINDOWING_X11)
		components->vpD.recordWindowXID = GDK_WINDOW_XID(window);
	#endif
}

static void delete_player_cb(GtkWidget *widget, GdkEvent *event, Components *components) {
	cleanup_vp(&components->vpD);
	gtk_widget_hide(components->player);
	gtk_button_set_label(GTK_BUTTON(components->vpButton), "Play Video");
}

void vp_open_file_cb(GtkWidget *widget, Components *components) {
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Select a video...", (GtkWindow*)components->player, 
													GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *argv[1];
		argv[0] = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		components->vpD.pauseImage = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
		gtk_button_set_image((GtkButton *)components->vpD.playPauseButton, components->vpD.pauseImage);
		cleanup_vp(&components->vpD);
		init_vp(1, argv, &components->vpD);
  	}

	gtk_widget_destroy(dialog);
}

void play_pause_button_cb(GtkWidget *widget, Components *components) {
	GtkWidget *curImage;
	
	if (components->vpD.pipeline == NULL)
		return;
	
	curImage = gtk_button_get_image((GtkButton *)widget);
	if (curImage == components->vpD.playImage) {
		components->vpD.pauseImage = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
		gtk_button_set_image((GtkButton *)widget, components->vpD.pauseImage);
		gst_element_set_state (components->vpD.pipeline, GST_STATE_PLAYING);

	} else if (curImage == components->vpD.pauseImage) {
		components->vpD.playImage = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
		gtk_button_set_image((GtkButton *)widget, components->vpD.playImage);
		gst_element_set_state (components->vpD.pipeline, GST_STATE_PAUSED);
	}
}

void rewind_button_cb(GtkWidget *widget, Components *components) {
	gint64 position;
	GstFormat format = GST_FORMAT_TIME;
	GstEvent *seek_event;
   	
	/* Obtain the current position, needed for the seek event */
	if (!gst_element_query_position(components->vpD.pipeline, &format, &position)) {
		g_printerr ("Unable to retrieve current position.\n");
		return;
	}

	if (components->vpD.playbackSpeed >= 0.0) {
		components->vpD.playbackSpeed = -1.0;
	} else {
		components->vpD.playbackSpeed = components->vpD.playbackSpeed * 2.0;
	}
	/* Demuxer is needed to rewind */
	seek_event = gst_event_new_seek(components->vpD.playbackSpeed, GST_FORMAT_TIME, 
									GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
									GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, position);
	/* Send the event */
  	gst_element_send_event(components->vpD.videoSink, seek_event);
   
  	g_print("Playback Speed: %f\n", components->vpD.playbackSpeed);
}

void forward_button_cb(GtkWidget *widget, Components *components) {
	gint64 position;
	GstFormat format = GST_FORMAT_TIME;
	GstEvent *seek_event;
   	
	/* Obtain the current position, needed for the seek event */
	if (!gst_element_query_position(components->vpD.pipeline, &format, &position)) {
		g_printerr ("Unable to retrieve current position.\n");
		return;
	}

	if (components->vpD.playbackSpeed <= 0.0) {
		components->vpD.playbackSpeed = 1.0;
	} else {
		components->vpD.playbackSpeed = components->vpD.playbackSpeed * 2.0;
	}
   
	/* Create the seek event */
	seek_event = gst_event_new_seek(components->vpD.playbackSpeed, GST_FORMAT_TIME, 
									GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, 
									GST_SEEK_TYPE_SET, position, GST_SEEK_TYPE_NONE, 0);
	/* Send the event */
  	gst_element_send_event(components->vpD.videoSink, seek_event);
   
  	g_print ("Playback Speed: %f\n", components->vpD.playbackSpeed);
}



void setup_vp_window(Components *components) {
	GtkWidget *controls;
	GtkWidget *actionArea;
	GtkWidget *image;

	components->player = gtk_dialog_new_with_buttons("CS414 Player", GTK_WINDOW(components->window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	gtk_window_set_transient_for(GTK_WINDOW(components->player), GTK_WINDOW(components->window));
	/* Center the player */
	gtk_window_set_position(GTK_WINDOW(components->player), GTK_WIN_POS_CENTER);
	/* Set default size for the player */
	gtk_window_set_default_size(GTK_WINDOW(components->player), 960, 720);

	components->vpWindow = gtk_drawing_area_new();
	g_signal_connect(components->vpWindow, "realize", G_CALLBACK(vp_realize_cb), components);	

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->player)->vbox), components->vpWindow, TRUE, TRUE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
	components->vpD.fileButton = gtk_button_new();
	gtk_button_set_image((GtkButton *)components->vpD.fileButton, image);
  	g_signal_connect(G_OBJECT(components->vpD.fileButton), "clicked", G_CALLBACK(vp_open_file_cb), components);

	components->vpD.pauseImage = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
	components->vpD.playPauseButton = gtk_button_new();
	gtk_button_set_image((GtkButton *)components->vpD.playPauseButton, components->vpD.pauseImage);
  	g_signal_connect (G_OBJECT(components->vpD.playPauseButton), "clicked", G_CALLBACK(play_pause_button_cb), components);
	
	image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_REWIND, GTK_ICON_SIZE_MENU);
  	components->vpD.rewindButton = gtk_button_new();
	gtk_button_set_image((GtkButton *)components->vpD.rewindButton, image);
  	g_signal_connect (G_OBJECT(components->vpD.rewindButton), "clicked", G_CALLBACK (rewind_button_cb), components);
	
	image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_FORWARD, GTK_ICON_SIZE_MENU);
  	components->vpD.forwardButton = gtk_button_new();
	gtk_button_set_image((GtkButton *)components->vpD.forwardButton, image);
  	g_signal_connect (G_OBJECT(components->vpD.forwardButton), "clicked", G_CALLBACK (forward_button_cb), components);

	controls = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(controls), components->vpD.fileButton, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(controls), components->vpD.playPauseButton, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (controls), components->vpD.rewindButton, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (controls), components->vpD.forwardButton, TRUE, TRUE, 2);
	
	actionArea = gtk_dialog_get_action_area(GTK_DIALOG(components->player));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->player)->vbox), controls, FALSE, FALSE, 3);
	
	g_signal_connect(components->player, "delete-event", G_CALLBACK(delete_player_cb), components);
}

char *audioArgv[4];

static void record_audio_cb(GtkWidget *widget, Components *components) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widget)), "Stop Audio Recording") == 0) {
		cleanup_ar(&components->arD);
		gtk_button_set_label(GTK_BUTTON(components->arButton), "Record Audio");
		return;
	}
	
	gtk_widget_show_all(components->arOptions);
}

void setup_ar_button(Components *components) {
	/* Creates a new button */
    components->arButton = gtk_button_new_with_label("Record Audio");
	/* Handle"clicked" signal */
    g_signal_connect(components->arButton, "clicked", G_CALLBACK(record_audio_cb), components);
}

/* This function gets called when currently selected item changes */
static void sample_rate_cb(GtkComboBox *combo) {
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text(combo);
	g_free(audioArgv[0]);
    audioArgv[0] = string;
}

/* This function gets called when currently selected item changes */
static void sample_size_cb(GtkComboBox *combo) {
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text(combo);
   	g_free(audioArgv[1]);
    audioArgv[1] = string;
}

/* This function gets called when currently selected item changes */
static void channels_cb(GtkComboBox *combo) {
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text(combo);
   	g_free(audioArgv[2]);
    audioArgv[2] = string;
}

/* This function gets called when currently selected item changes */
static void format_cb(GtkComboBox *combo, gpointer data) {
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text(combo);
    g_free(audioArgv[3]);
    audioArgv[3] = string;
}

static void ar_options_cb(GtkWidget *widget, Components *components) {
	gtk_widget_hide(components->arOptions);

	gtk_button_set_label(GTK_BUTTON(components->arButton), "Stop Audio Recording");

	if(init_ar(4, audioArgv, &components->arD) < 0) {
		gtk_button_set_label(GTK_BUTTON(components->arButton), "Record Audio");
	}
}

static void destroy_ar_options_cb(GtkWidget *widget) {
	gtk_widget_hide(widget);
}

void setup_ar_options(Components *components) {
	GtkWidget *frame;
    GtkWidget *combo;
	GtkWidget *button;
	gint i;

	char *rates[12] = {"8000", "9600", "11025", "12000", "16000", "22050", 
					  "24000", "32000", "44100", "48000", "88200", "96000"};
	char *sizes[4] = {"8", "16", "24", "32"};
	char *channels[4] = {"1-mono", "2-stereo"};
	char *formats[3] = {"none", "mulawenc", "alawenc"};

	audioArgv[0] = malloc(20);
	strcpy(audioArgv[0], "22050");
	audioArgv[1] = malloc(20);
	strcpy(audioArgv[1], "16");
	audioArgv[2] = malloc(20);
	strcpy(audioArgv[2], "1");
	audioArgv[3] = malloc(20);
	strcpy(audioArgv[3], "mulawenc");

	components->arOptions = gtk_dialog_new_with_buttons("Audio Options", GTK_WINDOW(components->window), GTK_DIALOG_MODAL, NULL);
	/* Center the menu of options */
	gtk_window_set_position(GTK_WINDOW(components->arOptions), GTK_WIN_POS_CENTER);

    /* Create frame */
    frame = gtk_frame_new( "Sample Rate" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->arOptions)->vbox), frame, TRUE, TRUE, 10);
    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add(GTK_CONTAINER(frame), combo);
	for (i = 0; i < 12; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX(combo), rates[i] );
		if (i == 5) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
    /* Connect signal to tour handler function */
    g_signal_connect( G_OBJECT( combo ), "changed", G_CALLBACK(sample_rate_cb), NULL);

	/* Create frame */
    frame = gtk_frame_new("Sample Size" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->arOptions)->vbox), frame, TRUE, TRUE, 10);
    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add(GTK_CONTAINER(frame), combo);
	for (i = 0; i < 4; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX(combo), sizes[i]);
		if (i == 1) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
    /* Connect signal to tour handler function */
    g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(sample_size_cb), NULL);

	/* Create frame */
    frame = gtk_frame_new( "Channels" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->arOptions)->vbox), frame, TRUE, TRUE, 10);
    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add(GTK_CONTAINER(frame), combo);
	for (i = 0; i < 2; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), channels[i]);
		if (i == 0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
    /* Connect signal to tour handler function */
    g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(channels_cb), NULL);

	/* Create frame */
    frame = gtk_frame_new( "Format" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(components->arOptions)->vbox), frame, TRUE, TRUE, 10);
    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add(GTK_CONTAINER(frame), combo);
	for (i = 0; i < 3; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), formats[i]);
		if (i == 1) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
    /* Connect signal to tour handler function */
    g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(format_cb), NULL);

	button = gtk_button_new_with_label("Start Audio Recording");
    gtk_box_pack_start(GTK_BOX (GTK_DIALOG(components->arOptions)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(ar_options_cb), components);

	g_signal_connect(components->arOptions, "delete-event", G_CALLBACK(destroy_ar_options_cb), NULL);
}

static void play_audio_cb(GtkWidget *widget, Components *components) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(components->apButton)), "Close Audio Player") == 0) {
		cleanup_ap(&components->apD);
		gtk_button_set_label(GTK_BUTTON(components->apButton), "Play Audio");
		return;
	}
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Select an audio...", (GtkWindow*)components->window, 
													GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      								GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	/* Center */
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *argv[1];
		argv[0] = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if (init_ap(1, argv, &components->apD) >= 0) {
			gtk_button_set_label(GTK_BUTTON(components->apButton), "Close Audio Player");
		}
  	}

	gtk_widget_destroy (dialog);
}

void setup_ap_button(Components *components) {
	/* Creates a new button */
    components->apButton = gtk_button_new_with_label("Play Audio");
	/* Handle"clicked" signal */
    g_signal_connect(components->apButton, "clicked", G_CALLBACK(play_audio_cb), components);
	
	components->apD.apButton = components->apButton;
}



