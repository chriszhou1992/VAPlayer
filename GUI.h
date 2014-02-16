#ifndef GUI_H_
#define GUI_H_

#include <gtk/gtk.h>

#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif

#include "vr.h"
#include "ar.h"
#include "vp.h"
#include "ap.h"

typedef struct _Components {
	/* GtkWidget is the storage type for widgets */
    GtkWidget *window;

	GtkWidget *mainBox;
	GtkWidget *optionsBox;
 	GtkWidget *recordButton;
	GtkWidget *audioButton;
	GtkWidget *recordOptions;
	GtkWidget *audioOptions;
	/* The drawing area where the recording will be shown */
	GtkWidget *recordWindow;
	GtkWidget *playerWindow;
	/* The drawing area where the playing will be shown */
	GtkWidget *playWindow;
	GtkWidget *fileButton, *playButton, *pauseButton, *rewindButton, *fastforwardButton;

	GtkWidget *vpButton;
	GtkWidget *apButton;

} Components;

static gboolean delete_event(GtkWidget *widget, GdkEvent *event);
static void destroy(GtkWidget *widget);
static void record_video(GtkWidget *widget);
static void record_audio(GtkWidget *widget);
static void realize(GtkWidget *widget);
static void destroy_audio_options();

void init_GUI(int argc, char *argv[]);
void setup_main_window();
void setup_record_button();
void setup_audio_button();
void setup_record_audio_options();
void setup_record_radio_buttons();
void setup_record_window();
void setup_player_window();
void setup_vp_button();
void setup_ap_button();
#endif
