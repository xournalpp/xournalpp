//
// Created by julius on 01.04.20.
//

#include "gtkdrawingareascrollable.h"

#define DRAWING_SCROLLABLE_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE((object), TYPE_DRAWING_SCROLLABLE, DrawingScrollablePrivate))

struct _DrawingScrollablePrivate {
    GtkAdjustment* hadjustment;
    GtkAdjustment* vadjustment;

    guint hscroll_policy : 1;
    guint vscroll_policy : 1;
};

enum { PROP_0, PROP_HADJUSTMENT, PROP_VADJUSTMENT, PROP_HSCROLL_POLICY, PROP_VSCROLL_POLICY };

G_DEFINE_TYPE_WITH_CODE(DrawingScrollable, drawing_scrollable, GTK_TYPE_DRAWING_AREA,
                        G_ADD_PRIVATE(DrawingScrollable) G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, NULL))

static void drawing_scrollable_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
    DrawingScrollable* viewport = DRAWING_SCROLLABLE(object);

    switch (prop_id) {
        case PROP_HADJUSTMENT:
            g_value_set_object(value, viewport->priv->hadjustment);
            break;
        case PROP_VADJUSTMENT:
            g_value_set_object(value, viewport->priv->vadjustment);
            break;
        case PROP_HSCROLL_POLICY:
            g_value_set_enum(value, viewport->priv->hscroll_policy);
            break;
        case PROP_VSCROLL_POLICY:
            g_value_set_enum(value, viewport->priv->vscroll_policy);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void drawing_scrollable_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
    DrawingScrollable* drawingScrollable = DRAWING_SCROLLABLE(object);
    switch (prop_id) {
        case PROP_HADJUSTMENT: {
            GtkAdjustment* h = GTK_ADJUSTMENT(g_value_get_object(value));
            if (!h)
                h = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            g_object_unref(drawingScrollable->priv->hadjustment);
            drawingScrollable->priv->hadjustment = h;
            g_object_ref_sink(h);
            break;
        }
        case PROP_VADJUSTMENT: {
            GtkAdjustment* v = GTK_ADJUSTMENT(g_value_get_object(value));
            if (!v)
                v = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            g_object_unref(drawingScrollable->priv->hadjustment);
            drawingScrollable->priv->vadjustment = v;
            g_object_ref_sink(v);
            break;
        }
        case PROP_HSCROLL_POLICY: {
            if (drawingScrollable->priv->hscroll_policy != g_value_get_enum(value)) {
                drawingScrollable->priv->hscroll_policy = g_value_get_enum(value);
                gtk_widget_queue_resize(GTK_WIDGET(drawingScrollable));
                g_object_notify_by_pspec(object, pspec);
            }
            break;
        }
        case PROP_VSCROLL_POLICY: {
            if (drawingScrollable->priv->vscroll_policy != g_value_get_enum(value)) {
                drawingScrollable->priv->vscroll_policy = g_value_get_enum(value);
                gtk_widget_queue_resize(GTK_WIDGET(drawingScrollable));
                g_object_notify_by_pspec(object, pspec);
            }
            break;
        }
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void drawing_scrollable_destroy(GtkWidget* widget) {
    DrawingScrollable* drawingScrollable = DRAWING_SCROLLABLE(widget);
    g_object_unref(drawingScrollable->priv->hadjustment);
    g_object_unref(drawingScrollable->priv->vadjustment);
    GTK_WIDGET_CLASS(drawing_scrollable_parent_class)->destroy(widget);
}

static void drawing_scrollable_class_init(DrawingScrollableClass* clazz) {
    GObjectClass* gObjectClass = G_OBJECT_CLASS(clazz);
    GtkWidgetClass* widgetClass = GTK_WIDGET_CLASS(clazz);

    widgetClass->destroy = drawing_scrollable_destroy;
    gObjectClass->get_property = drawing_scrollable_get_property;
    gObjectClass->set_property = drawing_scrollable_set_property;

    g_object_class_override_property(gObjectClass, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property(gObjectClass, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property(gObjectClass, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property(gObjectClass, PROP_VSCROLL_POLICY, "vscroll-policy");
}

static void drawing_scrollable_init(DrawingScrollable* drawingScrollable) {
    drawingScrollable->priv = DRAWING_SCROLLABLE_GET_PRIVATE(drawingScrollable);
    drawingScrollable->priv->hadjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    g_object_ref_sink(drawingScrollable->priv->hadjustment);
    drawingScrollable->priv->vadjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    g_object_ref_sink(drawingScrollable->priv->vadjustment);
}

GtkWidget* drawing_scrollable_new(void) { return GTK_WIDGET(g_object_new(TYPE_DRAWING_SCROLLABLE, nullptr)); }