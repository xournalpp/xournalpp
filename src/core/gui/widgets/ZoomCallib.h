/*
 * Xournal++
 *
 * Control to callibrate the zoom to fit the display DPI
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#pragma once

#include <glib-object.h>  // for G_TYPE_CHECK_INSTANCE_TYPE, G_TYPE_CHECK_IN...
#include <glib.h>         // for gint, G_BEGIN_DECLS, G_END_DECLS
#include <gtk/gtk.h>      // for GtkWidget, GtkWidgetClass

struct _ZoomCallib;
struct _ZoomCallibClass;

G_BEGIN_DECLS

#define ZOOM_CALLIB(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, zoomcallib_get_type(), ZoomCallib)
#define ZOOM_CALLIB_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, zoomcallib_get_type(), ZoomCallibClass)
#define IS_ZOOM_CALLIB(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, zoomcallib_get_type())

typedef struct _ZoomCallib ZoomCallib;
typedef struct _ZoomCallibClass ZoomCallibClass;

struct _ZoomCallib {
    GtkWidget widget;

    gint val;
};

struct _ZoomCallibClass {
    GtkWidgetClass parent_class;
};

GType zoomcallib_get_type(void);
void zoomcallib_set_val(ZoomCallib* callib, gint val);
GtkWidget* zoomcallib_new();

G_END_DECLS
