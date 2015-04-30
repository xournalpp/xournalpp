#include "SelectColor.h"
#include "math.h"

static void selectcolor_class_init(SelectColorClass* klass);
static void selectcolor_init(SelectColor* cpu);
static void selectcolor_size_request(GtkWidget* widget, GtkRequisition* requisition);
static void selectcolor_size_allocate(GtkWidget* widget, GtkAllocation* allocation);
static void selectcolor_realize(GtkWidget* widget);
static gboolean selectcolor_expose(GtkWidget* widget, GdkEventExpose* event);
static void selectcolor_paint(GtkWidget* widget);
static void selectcolor_destroy(GtkObject* object);

G_DEFINE_TYPE(SelectColor, selectcolor, GTK_TYPE_MISC)

GtkWidget* selectcolor_new(gint color)
{
	GtkWidget* w = GTK_WIDGET(gtk_type_new(selectcolor_get_type()));
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

	if (gtk_widget_is_drawable(GTK_WIDGET(sc)))
	{
		selectcolor_paint(sc);
	}
}

void selectcolor_set_circle(GtkWidget* sc, gboolean circle)
{
	g_return_if_fail(sc != NULL);
	g_return_if_fail(IS_SELECT_COLOR(sc));

	SELECT_COLOR(sc)->circle = circle;

	if (gtk_widget_is_drawable(GTK_WIDGET(sc)))
	{
		selectcolor_paint(sc);
	}
}

void selectcolor_set_size(GtkWidget* sc, gint size)
{
	g_return_if_fail(sc != NULL);
	g_return_if_fail(IS_SELECT_COLOR(sc));

	SELECT_COLOR(sc)->size = size;
}

static void selectcolor_class_init(SelectColorClass* klass)
{
	GtkWidgetClass* widget_class;
	GtkObjectClass* object_class;

	widget_class = (GtkWidgetClass*) klass;
	object_class = (GtkObjectClass*) klass;

	//	widget_class->realize = selectcolor_realize;
	widget_class->size_request = selectcolor_size_request;
	//	widget_class->size_allocate = selectcolor_size_allocate;
	widget_class->expose_event = selectcolor_expose;
	object_class->destroy = selectcolor_destroy;
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
	GTK_WIDGET_SET_FLAGS(sc, GTK_NO_WINDOW);

	gtk_widget_add_events(GTK_WIDGET(sc), GDK_ALL_EVENTS_MASK);

	g_signal_connect(sc, "expose-event", G_CALLBACK(exposeEvent), sc);
}

static void selectcolor_size_request(GtkWidget* widget,  GtkRequisition* requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SELECT_COLOR(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = SELECT_COLOR(widget)->size;
	requisition->height = SELECT_COLOR(widget)->size;
}

static void selectcolor_size_allocate(GtkWidget* widget, GtkAllocation* allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SELECT_COLOR(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED(widget))
	{
		gdk_window_move_resize(widget->window, allocation->x, allocation->y, allocation->width, allocation->height);
	}
}

static void selectcolor_realize(GtkWidget* widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SELECT_COLOR(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

	GtkWidget* parent = gtk_widget_get_parent(widget);
	widget->window = parent->window;
}

static gboolean selectcolor_expose(GtkWidget* widget, GdkEventExpose* event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(IS_SELECT_COLOR(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	selectcolor_paint(widget);

	return FALSE;
}

static void selectcolor_paint(GtkWidget* widget)
{
	cairo_t* cr;
	gdk_threads_enter();
	cr = gdk_cairo_create(widget->window);
	cairo_fill(cr);

	gint color = SELECT_COLOR(widget)->color;
	gint circle = SELECT_COLOR(widget)->circle;

	double r = ((color & 0xff0000) >> 16) / 255.0;
	double g = ((color & 0xff00) >> 8) / 255.0;
	double b = ((color & 0xff)) / 255.0;

	gboolean sensitiv = gtk_widget_is_sensitive(widget);

	int x = widget->allocation.x;
	int y = widget->allocation.y;
	int width = 10;

	if (!sensitiv)
	{
		r = r / 2 + 0.5;
		g = g / 2 + 0.5;
		b = b / 2 + 0.5;
	}

	double radius = 0;

	if (widget->allocation.width > widget->allocation.height)
	{
		width = widget->allocation.height;
		x += (widget->allocation.width - width) / 2;
		radius = widget->allocation.height / 2.0;
	}
	else
	{
		width = widget->allocation.width;
		y += (widget->allocation.height - width) / 2;
		radius = widget->allocation.width / 2.0;
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

	if (sensitiv)
	{
		cairo_set_source_rgb(cr, 0, 0, 0);
	}
	else
	{
		cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	}

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

	cairo_destroy(cr);
	gdk_threads_leave();
}

static void selectcolor_destroy(GtkObject* object)
{
	SelectColor* sc;
	SelectColorClass* klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(IS_SELECT_COLOR(object));

	sc = SELECT_COLOR(object);

	klass = (SelectColorClass*) gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy)
	{
		(*GTK_OBJECT_CLASS(klass)->destroy)(object);
	}
}
