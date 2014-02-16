#include "GUI.h"

Components widgets;
Data data;
aData audioData;
pData vpData;
apData apdata;
char *options[] = { "320|240|30|ffenc_mpeg4|", "320|240|30|jpegenc|", 
					"640|480|30|ffenc_mpeg4|", "640|480|30|jpegenc|", 
					"960|720|15|ffenc_mpeg4|", "960|720|15|jpegenc|" };

GtkWidget *buttons[6];


char *audioArgv[4];

static gboolean delete_event(GtkWidget *widget, GdkEvent *event) {
    
	/* If you return FALSE in the "delete-event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */

	g_print ("delete event occurred\n");

    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete-event". */

    return False;
}

static void destroy(GtkWidget *widget) {
    gtk_main_quit ();
}

static void play_video(GtkWidget *widget) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widgets.vpButton)), "Close Video Player") == 0) {
		cleanup_vp(&vpData);
		gtk_widget_hide(widgets.playerWindow);
		gtk_button_set_label(GTK_BUTTON(widgets.vpButton), "Play Video");
		return;
	}
	
	gtk_button_set_label(GTK_BUTTON(widgets.vpButton), "Close Video Player");
	gtk_window_set_position(GTK_WINDOW(widgets.playerWindow), GTK_WIN_POS_CENTER);
	gtk_widget_show_all(widgets.playerWindow);
}

static void play_audio(GtkWidget *widget) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widgets.apButton)), "Close Audio Player") == 0) {
		cleanup_ap(&apdata);
		//gtk_widget_hide(widgets.playerWindow);
		gtk_button_set_label(GTK_BUTTON(widgets.apButton), "Play Audio");
		return;
	}
	GtkWidget *dialog =  gtk_file_chooser_dialog_new("Select an audio...", (GtkWindow*)widgets.playerWindow, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  	{
		char *argv[1];
		argv[0] = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if (init_ap(1, argv, &apdata) >= 0) {
			gtk_button_set_label(GTK_BUTTON(widgets.apButton), "Close Audio Player");
		}
  	}

	gtk_widget_destroy (dialog);
	//gtk_window_set_position(GTK_WINDOW(widgets.playerWindow), GTK_WIN_POS_CENTER);
	//gtk_widget_show_all(widgets.playerWindow);
}

static void record_video(GtkWidget *widget) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widgets.recordButton)), "Stop Video Recording") == 0) {
		cleanup_vr(&data);
		gtk_window_resize(GTK_WINDOW(widgets.window), 150, 20);
		gtk_button_set_label(GTK_BUTTON(widgets.recordButton), "Record Video");
		return;
	}
	
	gtk_widget_show_all(widgets.recordOptions);
}

static void record_audio(GtkWidget *widget) {
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widgets.audioButton)), "Stop Audio Recording") == 0) {
		cleanup_ar(&audioData);
		gtk_button_set_label(GTK_BUTTON(widgets.audioButton), "Record Audio");
		audioArgv[0] = malloc(20);
		strcpy(audioArgv[0], "22050");
		audioArgv[1] = malloc(20);
		strcpy(audioArgv[1], "16");
		audioArgv[2] = malloc(20);
		strcpy(audioArgv[2], "1");
		audioArgv[3] = malloc(20);
		strcpy(audioArgv[3], "mulawenc");

		return;
	}	
	gtk_widget_show_all(widgets.audioOptions);
}

/* This function is called when the GUI toolkit creates the physical window that will hold the video. */
static void realize(GtkWidget *widget) {
	GdkWindow *window = gtk_widget_get_window(widget);

	if (!gdk_window_ensure_native(window))
		g_error ("Couldn't create native window needed for GstXOverlay!");

	/* Retrieve window handler from GDK */
	#if defined (GDK_WINDOWING_WIN32)
		data.recordWindowXID = (guintptr)GDK_WINDOW_HWND(window);
	#elif defined (GDK_WINDOWING_QUARTZ)
		data.recordWindowXID = gdk_quartz_window_get_nsview(window);
	#elif defined (GDK_WINDOWING_X11)
		data.recordWindowXID = GDK_WINDOW_XID(window);
	#endif
}

/* This function is called when the GUI toolkit creates the physical window that will hold the video. */
static void realize_play(GtkWidget *widget) {
	GdkWindow *window = gtk_widget_get_window(widget);

	if (!gdk_window_ensure_native(window))
		g_error ("Couldn't create native window needed for GstXOverlay!");

	/* Retrieve window handler from GDK */
	#if defined (GDK_WINDOWING_WIN32)
		vpData.recordWindowXID = (guintptr)GDK_WINDOW_HWND(window);
	#elif defined (GDK_WINDOWING_QUARTZ)
		vpData.recordWindowXID = gdk_quartz_window_get_nsview(window);
	#elif defined (GDK_WINDOWING_X11)
		vpData.recordWindowXID = GDK_WINDOW_XID(window);
	#endif
}

static void record_options_call_back() {
	gtk_widget_hide(widgets.recordOptions);
	
	int i;
	char *option;
	for (i = 0; i < (int)(sizeof(buttons)/sizeof(buttons[0])); i++) {
		if (gtk_toggle_button_get_active((GtkToggleButton *)buttons[i])) {
			option = options[i];
		}
	}
	g_print("%s\n", option);
	g_print ("Start recording...\n");
	gtk_button_set_label(GTK_BUTTON(widgets.recordButton), "Stop Video Recording");

	char *spec = option;
	char *temp = option;
	char *argv[4];
	i = 0;
	int count = 0;
	while (*temp != '\0') {
		g_print("%s\n", temp);
		if(*temp == '|') {
			argv[i] = malloc(count+1);
			strncpy(argv[i], spec, count);
			*(argv[i]+count) = '\0';
			g_print("%s\n", argv[i]); 
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
	gtk_widget_show(widgets.recordWindow);
	gtk_window_resize(GTK_WINDOW(widgets.window), width, height + 60);
	
	if(init_vr(4, argv, &data) < 0) {
		gtk_button_set_label(GTK_BUTTON(widgets.recordButton), "Record Video");
	}
}

static void audio_options_call_back() {
	gtk_widget_hide(widgets.audioOptions);

	g_print ("Start Audio recording...\n");
	gtk_button_set_label(GTK_BUTTON(widgets.audioButton), "Stop Audio Recording");

	if(init_ar(4, audioArgv, &audioData) < 0) {
		gtk_button_set_label(GTK_BUTTON(widgets.audioButton), "Record Audio");
	}
}

static void destroy_record_options() {
	gtk_widget_hide(widgets.recordOptions);
}

static void destroy_audio_options() {
	gtk_widget_hide(widgets.audioOptions);
}

void init_GUI(int argc, char *argv[]) {

    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init(&argc, &argv);

	/* Create main window */
    setup_main_window();
	/* Create button for record video */
	setup_record_button();
	/* Create button for record audio */
	setup_audio_button();
	/* Create a list of video recording options */
	setup_record_radio_buttons();
	/* Create a list of audio recording options */
	setup_record_audio_options();
	/* Create the window that shows the video recording */
	setup_record_window();
	/* Create the window that shows the video playing */
	setup_player_window();
	/* Create button for play video */
	setup_vp_button();
	/* Create button for play audio */
	setup_ap_button();
	
	/* Create a vertical box that will hold record window, record button, and audio button */
	widgets.mainBox = gtk_vbox_new(FALSE, 1);
	/* Put items into the box */
	gtk_box_pack_start(GTK_BOX(widgets.mainBox), widgets.recordWindow, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(widgets.mainBox), widgets.recordButton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(widgets.mainBox), widgets.audioButton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(widgets.mainBox), widgets.vpButton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(widgets.mainBox), widgets.apButton, FALSE, FALSE, 0);
	/* Add the box just created into the main window */
  	gtk_container_add(GTK_CONTAINER(widgets.window), widgets.mainBox);
	
	/* Create a horizontal box that holds menu */
	widgets.optionsBox = gtk_hbox_new(TRUE, 1);
	//gtk_box_pack_start(GTK_BOX(widgets.mainBox), widgets.optionsBox, FALSE, FALSE, 0);

	
	/* The final step is to display this newly created widget. */
    gtk_widget_show_all(widgets.window);

	/* Realize window now so that the video window gets created and we can
	 * obtain its XID before the pipeline is started up and the videosink
	 * asks for the XID of the window to render onto */
	gtk_widget_realize(widgets.recordWindow);
	gtk_widget_realize(widgets.playWindow);
	/* we should have the XID now */
	g_assert(data.recordWindowXID != 0);
	g_assert(vpData.recordWindowXID != 0);
	/* If not recording, do not show the record window */
	gtk_widget_hide(widgets.recordWindow);
	gtk_widget_hide(widgets.playWindow);

	/* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
}

void setup_main_window() {
	/* create a new window */
    widgets.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* When the window is given the "delete-event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect(widgets.window, "delete-event", G_CALLBACK(delete_event), NULL);
	/* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete-event" callback. */
    g_signal_connect(widgets.window, "destroy", G_CALLBACK (destroy), NULL);

	gtk_window_set_title(GTK_WINDOW(widgets.window), "CS414 MP1");
	gtk_window_set_default_size(GTK_WINDOW(widgets.window), 300, 20);
}

void setup_record_button() {
	/* Creates a new button */
    widgets.recordButton = gtk_button_new_with_label ("Record Video");
	/* When the button receives the "clicked" signal, it will call a
     * function passing it NULL as its argument. */
    g_signal_connect(widgets.recordButton, "clicked", G_CALLBACK(record_video), NULL);

}

void setup_audio_button() {
	/* Creates a new button */
    widgets.audioButton = gtk_button_new_with_label ("Record Audio");
	/* When the button receives the "clicked" signal, it will call a
     * function passing it NULL as its argument. */
    g_signal_connect(widgets.audioButton, "clicked", G_CALLBACK(record_audio), NULL);

}

void setup_vp_button() {
	/* Creates a new button */
    widgets.vpButton = gtk_button_new_with_label ("Play Video");
	/* When the button receives the "clicked" signal, it will call a
     * function passing it NULL as its argument. */
    g_signal_connect(widgets.vpButton, "clicked", G_CALLBACK(play_video), NULL);
}

void setup_ap_button() {
	/* Creates a new button */
    widgets.apButton = gtk_button_new_with_label ("Play Audio");
	/* When the button receives the "clicked" signal, it will call a
     * function passing it NULL as its argument. */
    g_signal_connect(widgets.apButton, "clicked", G_CALLBACK(play_audio), NULL);
	apdata.apButton = widgets.apButton;

}

void setup_record_window() {
	widgets.recordWindow = gtk_drawing_area_new();
	g_signal_connect(widgets.recordWindow, "realize", G_CALLBACK(realize), NULL);	
}

void play_cb() {
	gst_element_set_state (vpData.pipeline, GST_STATE_PLAYING);
}

void pause_cb() {
	gst_element_set_state (vpData.pipeline, GST_STATE_PAUSED);
}
void rewind_cb() {
	gint64 position;
	GstFormat format = GST_FORMAT_TIME;
	GstEvent *seek_event;
   	
	if (vpData.rate > 0.0) {
		vpData.rate = -1.0;
	} else {
		vpData.rate = vpData.rate * 2.0;
	}
	/* Obtain the current position, needed for the seek event */
	if (!gst_element_query_position (vpData.pipeline, &format, &position)) {
		g_printerr ("Unable to retrieve current position.\n");
		return;
	}
	/* Create the seek event */
	/* Demuxer is needed */
	seek_event = gst_event_new_seek (
			vpData.rate, GST_FORMAT_TIME, GST_SEEK_FLAG_SKIP,
        GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, position);
	/* Send the event */
  	gst_element_send_event (vpData.videoSink, seek_event);
   
  	g_print ("Current rate: %g\n", vpData.rate);
}

void fastforward_cb() {
	gint64 position;
	GstFormat format = GST_FORMAT_TIME;
	GstEvent *seek_event;
   	
	if (vpData.rate < 0.0) {
		vpData.rate = 1.0;
	} else {
		vpData.rate = vpData.rate * 2.0;
	}
	/* Obtain the current position, needed for the seek event */
	if (!gst_element_query_position (vpData.pipeline, &format, &position)) {
		g_printerr ("Unable to retrieve current position.\n");
		return;
	}
   
	/* Create the seek event */
	
	seek_event = gst_event_new_seek (vpData.rate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | 						GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET, position, GST_SEEK_TYPE_NONE, 0);
	/* Send the event */
  	gst_element_send_event (vpData.videoSink, seek_event);
   
  	g_print ("Current rate: %g\n", vpData.rate);
}

void open_file_cb() {
	GtkWidget *dialog =  gtk_file_chooser_dialog_new("Select a video...", (GtkWindow*)widgets.playerWindow, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  	{
		char *argv[1];
		argv[0] = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		init_vp(1, argv, &vpData);
  	}

	gtk_widget_destroy (dialog);
}

void setup_player_window() {
	GtkWidget *controls;
	GtkWidget *actionArea;

	vpData.rate = 1.0;

	widgets.playerWindow = gtk_dialog_new_with_buttons("CS414 Player", GTK_WINDOW(widgets.window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	gtk_window_set_transient_for(GTK_WINDOW(widgets.playerWindow),GTK_WINDOW(widgets.window));
	gtk_window_set_default_size(GTK_WINDOW(widgets.playerWindow), 960, 720);

	widgets.playWindow = gtk_drawing_area_new();
	g_signal_connect(widgets.playWindow, "realize", G_CALLBACK(realize_play), NULL);	

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.playerWindow)->vbox), widgets.playWindow, TRUE, TRUE, 0);

	widgets.fileButton = gtk_button_new_from_stock(GTK_STOCK_DIRECTORY);
  	g_signal_connect (G_OBJECT(widgets.fileButton), "clicked", G_CALLBACK(open_file_cb), NULL);

	widgets.playButton = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
  	g_signal_connect (G_OBJECT(widgets.playButton), "clicked", G_CALLBACK(play_cb), NULL);
	
   
  	widgets.pauseButton = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PAUSE);
  	g_signal_connect (G_OBJECT (widgets.pauseButton), "clicked", G_CALLBACK (pause_cb), NULL);
	
   	
  	widgets.rewindButton = gtk_button_new_from_stock(GTK_STOCK_MEDIA_REWIND);
  	g_signal_connect (G_OBJECT (widgets.rewindButton), "clicked", G_CALLBACK (rewind_cb), NULL);
	
	widgets.fastforwardButton = gtk_button_new_from_stock(GTK_STOCK_MEDIA_FORWARD);
  	g_signal_connect (G_OBJECT (widgets.fastforwardButton), "clicked", G_CALLBACK (fastforward_cb), NULL);
	

	controls = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (controls), widgets.fileButton, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (controls), widgets.playButton, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (controls), widgets.pauseButton, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (controls), widgets.rewindButton, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (controls), widgets.fastforwardButton, TRUE, TRUE, 2);

	actionArea = gtk_dialog_get_action_area(GTK_DIALOG(widgets.playerWindow));
	//gtk_widget_destroy(actionArea);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.playerWindow)->vbox), controls, FALSE, FALSE, 3);

	g_signal_connect(widgets.playerWindow, "delete-event", G_CALLBACK(play_video), NULL);
}

void setup_record_radio_buttons() {
	GtkWidget *button;
    GSList *group;

	widgets.recordOptions = gtk_dialog_new_with_buttons("Video Options", GTK_WINDOW(widgets.window), GTK_DIALOG_MODAL, NULL);

    buttons[0] = gtk_radio_button_new_with_label(NULL, "Resolution: 320 x 240 | Frame Rate: 30 | Compression Format: mpeg4");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.recordOptions)->vbox), buttons[0], TRUE, TRUE, 0);

	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(buttons[0]));
    buttons[1] = gtk_radio_button_new_with_label(group, "Resolution: 320 x 240 | Frame Rate: 30 | Compression Format: mjpeg");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.recordOptions)->vbox), buttons[1], TRUE, TRUE, 0);

	buttons[2] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[1]), "Resolution: 640 x 480 | Frame Rate: 30 | Compression Format: mpeg4");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[2]), TRUE);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.recordOptions)->vbox), buttons[2], TRUE, TRUE, 0);
	

    buttons[3] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[2]), "Resolution: 640 x 480 | Frame Rate: 30 | Compression Format: mjpeg");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.recordOptions)->vbox), buttons[3], TRUE, TRUE, 0);

	buttons[4] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[3]), "Resolution: 960 x 720 | Frame Rate: 15 | Compression Format: mpeg4");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.recordOptions)->vbox), buttons[4], TRUE, TRUE, 0);

	buttons[5] = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (buttons[4]), "Resolution: 960 x 720 | Frame Rate: 15 | Compression Format: mjpeg");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.recordOptions)->vbox), buttons[5], TRUE, TRUE, 0);
    

    button = gtk_button_new_with_label("Start Video Recording");
    gtk_box_pack_start(GTK_BOX (GTK_DIALOG(widgets.recordOptions)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(record_options_call_back), NULL);
    //gtk_widget_set_can_default(button, TRUE);
    //gtk_widget_grab_default(button);

	gtk_window_set_default_size(GTK_WINDOW(widgets.recordOptions), 320, 240);
	g_signal_connect(widgets.recordOptions, "delete-event", G_CALLBACK(destroy_record_options), NULL);
}

/* This function gets called when currently selected item changes */
static void sample_rate_change( GtkComboBox *combo, gpointer data )
{
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text( combo );
	g_free( audioArgv[0] );
    audioArgv[0] = string;
}

/* This function gets called when currently selected item changes */
static void sample_size_change( GtkComboBox *combo, gpointer data )
{
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text( combo );

   	g_free( audioArgv[1] );
    audioArgv[1] = string;
}

/* This function gets called when currently selected item changes */
static void channels_change( GtkComboBox *combo, gpointer data )
{
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text( combo );

   	g_free( audioArgv[2] );
    audioArgv[2] = string;
}

/* This function gets called when currently selected item changes */
static void format_change( GtkComboBox *combo, gpointer data )
{
    /* Obtain currently selected string from combo box */
    gchar *string = gtk_combo_box_get_active_text( combo );

    g_free( audioArgv[3] );
    audioArgv[3] = string;
}

void setup_record_audio_options() {
	GtkWidget *frame;
    GtkWidget *combo;
	gint i;
	GtkWidget *button;

	char *rates[12] = {"8000", "9600", "11025", "12000", "16000", "22050", 
					  "24000", "32000", "44100", "48000", "88200", "96000"};
	//char *sizes[4] = {"8", "16", "24", "32"};
	char *sizes[1] = {"16"};
	char *channels[4] = {"1-mono", "2-stereo"};
	char *formats[2] = {"mulawenc", "alawenc"};

	audioArgv[0] = malloc(20);
	strcpy(audioArgv[0], "22050");
	audioArgv[1] = malloc(20);
	strcpy(audioArgv[1], "16");
	audioArgv[2] = malloc(20);
	strcpy(audioArgv[2], "1");
	audioArgv[3] = malloc(20);
	strcpy(audioArgv[3], "mulawenc");

	widgets.audioOptions = gtk_dialog_new_with_buttons("Audio Options", GTK_WINDOW(widgets.window), GTK_DIALOG_MODAL, NULL);

     /* Create frame */
    frame = gtk_frame_new( "Sample Rate" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.audioOptions)->vbox), frame, TRUE, TRUE, 10);

    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add( GTK_CONTAINER( frame ), combo );
	for (i = 0; i < 12; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX( combo ), rates[i] );
		if (i == 5) {
			gtk_combo_box_set_active(GTK_COMBO_BOX( combo ), i);
		}
	}

    /* Connect signal to tour handler function */
    g_signal_connect( G_OBJECT( combo ), "changed", G_CALLBACK( sample_rate_change ), NULL );

	/* Create frame */
    frame = gtk_frame_new( "Sample Size" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.audioOptions)->vbox), frame, TRUE, TRUE, 10);

    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add( GTK_CONTAINER( frame ), combo );
	for (i = 0; i < 1; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX( combo ), sizes[i] );
		if (i == 0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX( combo ), i);
		}
	}

    /* Connect signal to tour handler function */
    g_signal_connect( G_OBJECT( combo ), "changed", G_CALLBACK( sample_size_change ), NULL );

	/* Create frame */
    frame = gtk_frame_new( "Channels" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.audioOptions)->vbox), frame, TRUE, TRUE, 10);

    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add( GTK_CONTAINER( frame ), combo );
	for (i = 0; i < 2; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX( combo ), channels[i] );
		if (i == 0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX( combo ), i);
		}
	}

    /* Connect signal to tour handler function */
    g_signal_connect( G_OBJECT( combo ), "changed", G_CALLBACK( channels_change ), NULL );

	/* Create frame */
    frame = gtk_frame_new( "Format" );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(widgets.audioOptions)->vbox), frame, TRUE, TRUE, 10);

    /* Create combo box using text API and add some data to it */
    combo = gtk_combo_box_new_text();
    gtk_container_add( GTK_CONTAINER( frame ), combo );
	for (i = 0; i < 2; i++) {
		gtk_combo_box_append_text( GTK_COMBO_BOX( combo ), formats[i] );
		if (i == 0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX( combo ), i);
		}
	}

    /* Connect signal to tour handler function */
    g_signal_connect( G_OBJECT( combo ), "changed", G_CALLBACK( format_change ), NULL );

	button = gtk_button_new_with_label("Start Audio Recording");
    gtk_box_pack_start(GTK_BOX (GTK_DIALOG(widgets.audioOptions)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(audio_options_call_back), NULL);

	gtk_window_set_default_size(GTK_WINDOW(widgets.recordOptions), 320, 240);
	g_signal_connect(widgets.recordOptions, "delete-event", G_CALLBACK(destroy_audio_options), NULL);
}

