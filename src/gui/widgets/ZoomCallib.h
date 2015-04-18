/*
 * Xournal++
 *
 * Control to callibrate the zoom to fit the display DPI
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define ZOOM_CALLIB(obj) GTK_CHECK_CAST(obj, zoomcallib_get_type(), ZoomCallib)
#define ZOOM_CALLIB_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, zoomcallib_get_type(), ZoomCallibClass)
#define IS_ZOOM_CALLIB(obj) GTK_CHECK_TYPE(obj, zoomcallib_get_type())

typedef struct _ZoomCallib ZoomCallib;
typedef struct _ZoomCallibClass ZoomCallibClass;

struct _ZoomCallib
{
	GtkWidget widget;

	gint val;
};

struct _ZoomCallibClass
{
	GtkWidgetClass parent_class;
};

GtkType zoomcallib_get_type(void);
void zoomcallib_set_val(ZoomCallib* cpu, gint val);
GtkWidget* zoomcallib_new();

G_END_DECLS
