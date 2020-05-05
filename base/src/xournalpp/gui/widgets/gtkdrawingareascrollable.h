//
// Created by julius on 01.04.20.
//

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_DRAWING_SCROLLABLE (gtk_drawing_scrollable_get_type())
#define GTK_DRAWING_SCROLLABLE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_DRAWING_SCROLLABLE, GtkDrawingScrollable))
#define GTK_DRAWING_SCROLLABLE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_DRAWING_SCROLLABLE, GtkDrawingScrollableClass))
#define GTK_IS_DRAWING_SCROLLABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_DRAWING_SCROLLABLE))
#define GTK_IS_DRAWING_SCROLLABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_DRAWING_SCROLLABLE))
#define GTK_DRAWING_SCROLLABLE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_DRAWING_SCROLLABLE, GtkDrawingScrollableClass))


typedef struct _GtkDrawingScrollable GtkDrawingScrollable;
typedef struct _GtkDrawingScrollablePrivate GtkDrawingScrollablePrivate;
typedef struct _GtkDrawingScrollableClass GtkDrawingScrollableClass;

struct _GtkDrawingScrollable {
    GtkDrawingArea area;

    /*< private >*/
    GtkDrawingScrollablePrivate* priv;
};

struct _GtkDrawingScrollableClass {
    GtkDrawingAreaClass parent_class;

    /* Padding for future expansion */
    void (*_gtk_reserved1)(void);
    void (*_gtk_reserved2)(void);
    void (*_gtk_reserved3)(void);
    void (*_gtk_reserved4)(void);
};

GType gtk_drawing_scrollable_get_type(void) G_GNUC_CONST;
GtkWidget* gtk_drawing_scrollable_new(void);

G_END_DECLS
