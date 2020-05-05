//
// Created by julius on 01.04.20.
//

#include "gtkdrawingareascrollable.h"

#define GTK_DRAWING_SCROLLABLE_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_DRAWING_SCROLLABLE, GtkDrawingScrollablePrivate))

struct _GtkDrawingScrollablePrivate {
    GtkAdjustment* hadjustment;
    GtkAdjustment* vadjustment;

    guint hscroll_policy : 1;
    guint vscroll_policy : 1;
};

enum { PROP_0, PROP_HADJUSTMENT, PROP_VADJUSTMENT, PROP_HSCROLL_POLICY, PROP_VSCROLL_POLICY };

G_DEFINE_TYPE_WITH_CODE(GtkDrawingScrollable, gtk_drawing_scrollable, GTK_TYPE_DRAWING_AREA,
                        G_ADD_PRIVATE(GtkDrawingScrollable) G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, NULL))

static void gtk_drawing_scrollable_class_init(GtkDrawingScrollableClass* clazz) {
    GObjectClass* gObjectClass = G_OBJECT_CLASS(clazz);
    g_object_class_override_property(gObjectClass, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property(gObjectClass, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property(gObjectClass, PROP_HSCROLL_POLICY, "hscroll_policy");
    g_object_class_override_property(gObjectClass, PROP_VSCROLL_POLICY, "vscroll_policy");
}

static void gtk_drawing_scrollable_init(GtkDrawingScrollable* drawingScrollable) {
    drawingScrollable->priv = GTK_DRAWING_SCROLLABLE_GET_PRIVATE(drawingScrollable);
    // drawingScrollable->priv->hadjustment = nullptr;
    // drawingScrollable->priv->vadjustment = nullptr;
}

GtkWidget* gtk_drawing_scrollable_new() { return GTK_WIDGET(g_object_new(GTK_TYPE_DRAWING_SCROLLABLE, nullptr)); }