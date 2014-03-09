#include "gui.h"

void init_gui(int argc, char *argv[]) {

	Components components;

    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init(0, NULL);

	/* Create main window */
    setup_main_window(&components);

	/* Create button for video recording */
	setup_vr_button(&components);
	/* Create a list of video recording options */
	setup_vr_options(&components);
	/* Create a window that shows the video recording */
	setup_vr_window(&components);

	/* Create button for play video */
	setup_vp_button(&components);
	/* Create a window that shows the video playing */
	setup_vp_window(&components);

	/* Create button for audio recording */
	setup_ar_button(&components);
	/* Create a list of audio recording options */
	setup_ar_options(&components);

	/* Create button for play audio */
	setup_ap_button(&components);

	/* Create a vertical box that will hold buttons just created */
	components.actionsBox = gtk_vbox_new(FALSE, 1);
	/* Put buttons into the box */
	gtk_box_pack_start(GTK_BOX(components.actionsBox), components.vrButton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(components.actionsBox), components.vpButton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(components.actionsBox), components.arButton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(components.actionsBox), components.apButton, FALSE, FALSE, 0);
	/* Put the box into the main window */
  	gtk_container_add(GTK_CONTAINER(components.window), components.actionsBox);
	
	/* Show the main window */
    gtk_widget_show_all(components.window);

	/* Realize window now so that the video window gets created and we can
	 * obtain its XID before the pipeline is started up and the videosink
	 * asks for the XID of the window to render onto */
	gtk_widget_realize(components.vrWindow);
	gtk_widget_realize(components.vpWindow);
	/* Check that we have the XID now */
	g_assert(components.vrD.recordWindowXID != 0);
	g_assert(components.vpD.recordWindowXID != 0);


	/* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur. */
    gtk_main ();
}

