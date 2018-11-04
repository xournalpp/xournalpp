#include "SelectColor.h"
#include "math.h"

static void selectcolor_class_init(SelectColorClass* klass);
static void selectcolor_init(SelectColor* cpu);
static void selectcolor_size_request(GtkWidget* widget,
                                     GtkRequisition* requisition);
static void selectcolor_size_allocate(GtkWidget* widget,
                                      GtkAllocation* allocation);
static void selectcolor_realize(GtkWidget* widget);
static gboolean selectcolor_draw(GtkWidget* widget, cairo_t* cr);
static gboolean selectcolor_expose(GtkWidget* widget, GdkEventExpose* event);
static void selectcolor_paint(GtkWidget* widget);
static void selectcolor_destroy(GtkWidget* object);

G_DEFINE_TYPE (SelectColor, selectcolor, GTK_TYPE_MISC)

GtkWidget* selectcolor_new(gint color)
{
	GtkWidget* w = GTK_WIDGET(g_object_new(selectcolor_get_type(), NULL));
	selectcolor_set_color(w, color);
	SELECT_COLOR(w)->circle = false;
	SELECT_COLOR(w)->size = 16;

	return w;
}

void selectcolor_set_color(GtkWidget* sc, gint color)
{
	g_return_if_fail(sc != NULL);
	g_return_if_fail(IS_SELECT_COLOR(sc));

	SELECT_COLOR(sc)->color = color;

	gtk_widget_queue_draw(sc);
}

void selectcolor_set_circle(GtkWidget* sc, gboolean circle)
{
	g_return_if_fail(sc != NULL);
	g_return_if_fail(IS_SELECT_COLOR(sc));

	SELECT_COLOR(sc)->circle = circle;

	gtk_widget_queue_draw(sc);
}

void selectcolor_set_size(GtkWidget* sc, gint size)
{
	g_return_if_fail(sc != NULL);
	g_return_if_fail(IS_SELECT_COLOR(sc));

	SELECT_COLOR(sc)->size = size;
}

static void
selectcolor_get_preferred_width(GtkWidget *widget,
                                gint* minimal_width,
                                gint* natural_width)
{
  GtkRequisition requisition;

  selectcolor_size_request(widget, &requisition);

  *minimal_width = *natural_width = requisition.width;
}

static void
selectcolor_get_preferred_height(GtkWidget* widget,
                                 gint* minimal_height,
                                 gint* natural_height)
{
  GtkRequisition requisition;

  selectcolor_size_request(widget, &requisition);

  *minimal_height = *natural_height = requisition.height;
}

static void selectcolor_class_init(SelectColorClass* klass)
{
	GtkWidgetClass* widget_class;

	widget_class = (GtkWidgetClass*) klass;

#if GTK3_ENABLED
	widget_class->destroy = selectcolor_destroy;
	widget_class->get_preferred_width = selectcolor_get_preferred_width;
	widget_class->get_preferred_height = selectcolor_get_preferred_height;
	widget_class->draw = selectcolor_draw;
#else
	widget_class->size_request = selectcolor_size_request;
	widget_class->expose_event = selectcolor_expose;

	GtkObjectClass* object_class;
	object_class = (GtkObjectClass*) klass;

	typedef void (*DestroyFunc) (GtkObject *object);

	object_class->destroy =  (DestroyFunc)selectcolor_destroy;
#endif
}

gboolean exposeEvent(GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(IS_SELECT_COLOR(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	selectcolor_paint(widget);

	return FALSE;
}

static void selectcolor_init(SelectColor* sc)
{
	sc->color = 0xffffffff;
	gtk_widget_set_has_window(GTK_WIDGET(sc), FALSE);

	gtk_widget_add_events(GTK_WIDGET(sc), GDK_ALL_EVENTS_MASK);

#if !GTK3_ENABLED
	g_signal_connect(sc, "expose-event", G_CALLBACK(exposeEvent), sc);
#endif
}

static void selectcolor_size_request(GtkWidget* widget,
                                     GtkRequisition* requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SELECT_COLOR(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = SELECT_COLOR(widget)->size;
	requisition->height = SELECT_COLOR(widget)->size;
}

static void selectcolor_size_allocate(GtkWidget* widget,
                                      GtkAllocation* allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SELECT_COLOR(widget));
	g_return_if_fail(allocation != NULL);

	gtk_widget_set_allocation(widget, allocation);

	if (gtk_widget_get_realized(widget))
	{
		gdk_window_move_resize(gtk_widget_get_window(widget),
		                       allocation->x, allocation->y,
		                       allocation->width, allocation->height);
	}
}

static void selectcolor_realize(GtkWidget* widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;
	GtkAllocation allocation;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SELECT_COLOR(widget));

	gtk_widget_set_realized(widget, TRUE);

	gtk_widget_get_allocation(widget, &allocation);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = allocation.x;
	attributes.y = allocation.y;
	attributes.width = allocation.width;
	attributes.height = allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget),
	                                             &attributes, attributes_mask));

	gdk_window_set_user_data(gtk_widget_get_window(widget), widget);

	gtk_widget_style_attach(widget);

	gtk_style_set_background(gtk_widget_get_style(widget),
	                         gtk_widget_get_window(widget),
	                         GTK_STATE_NORMAL);

	GtkWidget* parent = gtk_widget_get_parent(widget);
	gtk_widget_set_window(widget, gtk_widget_get_window(parent));
}

static void selectcolor_paint(GtkWidget* widget)
{
	gdk_threads_enter();
	cairo_t* cr = gdk_cairo_create(gtk_widget_get_window(widget));

	selectcolor_draw(widget, cr);

	cairo_destroy(cr);
	gdk_threads_leave();
}

static gboolean selectcolor_expose(GtkWidget* widget, GdkEventExpose* event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(IS_SELECT_COLOR(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	selectcolor_paint(widget);

	return FALSE;
}

static gboolean selectcolor_draw(GtkWidget* widget, cairo_t* cr)
{
	GtkAllocation allocation;

	gdk_threads_enter();
	cairo_fill(cr);

	gint color = SELECT_COLOR(widget)->color;
	gint circle = SELECT_COLOR(widget)->circle;

	double r = ((color & 0xff0000) >> 16) / 255.0;
	double g = ((color & 0xff00) >> 8) / 255.0;
	double b = ((color & 0xff)) / 255.0;
	gtk_widget_get_allocation(widget, &allocation);

	int x = 0;
	int y = 0;
	int width = 10;

	double radius = 0;

	if (allocation.width > allocation.height)
	{
		width = allocation.height;
		x += (allocation.width - width) / 2;
		radius = allocation.height / 2.0;
	}
	else
	{
		width = allocation.width;
		y += (allocation.height - width) / 2;
		radius = allocation.width / 2.0;
	}

	cairo_set_source_rgb(cr, r, g, b);

	if (circle)
	{
		cairo_arc(cr, radius + x, radius + y, radius - 1, 0, 2 * M_PI);
	}
	else
	{
		cairo_rectangle(cr, x + 1, y + 1, width - 2, width - 2);
	}
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0, 0, 0);

	if (circle)
	{
		cairo_arc(cr, radius + x, radius + y, radius - 1, 0, 2 * M_PI);
	}
	else
	{
		cairo_rectangle(cr, x + 1, y + 1, width - 2, width - 2);
	}

	cairo_set_line_width(cr, 0.8);
	cairo_stroke(cr);

	return true;
}

static void selectcolor_destroy(GtkWidget* object)
{
	/*
	 * TODO: how to destroy this object?
	SelectColor* sc;
	SelectColorClass* klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(IS_SELECT_COLOR(object));

	sc = SELECT_COLOR(object);

	klass = (SelectColorClass*) gtk_type_class(gtk_widget_get_type());

	if (GTK_WIDGET_CLASS(klass)->destroy)
	{
		(*GTK_WIDGET_CLASS(klass)->destroy)(object);
	}
	*/
}
