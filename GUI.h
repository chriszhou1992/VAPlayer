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

    GtkWidget *window;
	GtkWidget *actionsBox;

	GtkWidget *vrButton;
	GtkWidget *vrOptions;
	GtkWidget *recorder;
	GtkWidget *vrWindow;

	GtkWidget *vpButton;
	GtkWidget *player;
	GtkWidget *vpWindow;

	GtkWidget *arButton;
	GtkWidget *arOptions;
	
	GtkWidget *apButton;

	vrData vrD;
	vpData vpD;
	arData arD;
	apData apD;
} Components;


void init_GUI(int argc, char *argv[]);


#endif
