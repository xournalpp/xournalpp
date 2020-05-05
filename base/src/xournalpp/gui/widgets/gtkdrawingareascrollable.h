//
// Created by julius on 01.04.20.
//

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_DRAWING_SCROLLABLE (drawing_scrollable_get_type())
#define DRAWING_SCROLLABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DRAWING_SCROLLABLE, DrawingScrollable))
#define DRAWING_SCROLLABLE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_DRAWING_SCROLLABLE, DrawingScrollableClass))
#define IS_DRAWING_SCROLLABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DRAWING_SCROLLABLE))
#define IS_DRAWING_SCROLLABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_DRAWING_SCROLLABLE))
#define DRAWING_SCROLLABLE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_DRAWING_SCROLLABLE, DrawingScrollableClass))


typedef struct _DrawingScrollable DrawingScrollable;
typedef struct _DrawingScrollablePrivate DrawingScrollablePrivate;
typedef struct _DrawingScrollableClass DrawingScrollableClass;

struct _DrawingScrollable {
    GtkDrawingArea area;

    /*< private >*/
    DrawingScrollablePrivate* priv;
};

struct _DrawingScrollableClass {
    GtkDrawingAreaClass parent_class;

    /* Padding for future expansion */
    void (*_reserved1)(void);
    void (*_reserved2)(void);
    void (*_reserved3)(void);
    void (*_reserved4)(void);
};

GType drawing_scrollable_get_type(void) G_GNUC_CONST;
GtkWidget* drawing_scrollable_new(void);

G_END_DECLS
