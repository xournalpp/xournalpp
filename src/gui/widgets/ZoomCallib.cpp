#include "ZoomCallib.h"

static void zoomcallib_class_init(ZoomCallibClass* klass);
static void zoomcallib_init(ZoomCallib* callib);
static void zoomcallib_size_request(GtkWidget* widget,
                                    GtkRequisition* requisition);
static void zoomcallib_size_allocate(GtkWidget* widget,
                                     GtkAllocation* allocation);
static void zoomcallib_realize(GtkWidget* widget);
static gboolean zoomcallib_expose(GtkWidget* widget, GdkEventExpose* event);
static void zoomcallib_paint(GtkWidget* widget);
static void zoomcallib_destroy(GtkObject* object);

GtkType zoomcallib_get_type(void)
{
	static GtkType zoomcallib_type = 0;

	if (!zoomcallib_type)
	{
		static const GtkTypeInfo zoomcallib_info =
		{
			"ZoomCallib",
			sizeof(ZoomCallib),
			sizeof(ZoomCallibClass),
			(GtkClassInitFunc) zoomcallib_class_init,
			(GtkObjectInitFunc) zoomcallib_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		zoomcallib_type = gtk_type_unique(GTK_TYPE_WIDGET, &zoomcallib_info);
	}

	return zoomcallib_type;
}

void zoomcallib_set_val(ZoomCallib* callib, gint val)
{
	callib->val = val;

	if (gtk_widget_is_drawable(GTK_WIDGET(callib)))
	{
		zoomcallib_paint(GTK_WIDGET(callib));
	}
}

GtkWidget* zoomcallib_new()
{
	return GTK_WIDGET(gtk_type_new(zoomcallib_get_type()));
}

static void zoomcallib_class_init(ZoomCallibClass* klass)
{
	GtkWidgetClass* widget_class;
	GtkObjectClass* object_class;

	widget_class = (GtkWidgetClass*) klass;
	object_class = (GtkObjectClass*) klass;

	widget_class->realize = zoomcallib_realize;
	widget_class->size_request = zoomcallib_size_request;
	widget_class->size_allocate = zoomcallib_size_allocate;
	widget_class->expose_event = zoomcallib_expose;

	object_class->destroy = zoomcallib_destroy;
}

static void zoomcallib_init(ZoomCallib* zc)
{
	zc->val = 72;
}

static void zoomcallib_size_request(GtkWidget* widget,
                                    GtkRequisition* requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_ZOOM_CALLIB(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 200;
	requisition->height = 75;
}

static void zoomcallib_size_allocate(GtkWidget* widget,
                                     GtkAllocation* allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_ZOOM_CALLIB(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED(widget))
	{
		gdk_window_move_resize(widget->window, allocation->x, allocation->y,
		                       allocation->width, allocation->height);
	}
}

static void zoomcallib_realize(GtkWidget* widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_ZOOM_CALLIB(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
	                                &attributes, attributes_mask);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

static gboolean zoomcallib_expose(GtkWidget* widget, GdkEventExpose* event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(IS_ZOOM_CALLIB(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	zoomcallib_paint(widget);

	return FALSE;
}

static void zoomcallib_paint(GtkWidget* widget)
{
	cairo_t* cr;
	cairo_text_extents_t extents;

	cr = gdk_cairo_create(widget->window);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);

	gdouble hafCm = (ZOOM_CALLIB(widget)->val / 2.54) / 2;

	int h = widget->allocation.height;
	int heigth = 50;
	if (h < heigth)
	{
		heigth = widget->allocation.height - 10;
	}

	int i = 0;

	cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL,
	                       CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 13);

	for (gdouble x = 2; x < widget->allocation.width; x += hafCm, i++)
	{
		int y;
		if (i % 2 == 0)
		{
			cairo_set_source_rgb(cr, 0, 0, 0);
			y = heigth - 3;
		}
		else
		{
			cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
			y = heigth - 17;
		}

		cairo_rectangle(cr, x,  2 + h - y, 1, y);

		cairo_fill(cr);

		if (i % 2 == 0 && i != 0 && x < widget->allocation.width - 20)
		{
			cairo_set_source_rgb(cr, 0, 0, 0);

			char* txt = g_strdup_printf("%i", i / 2);
			cairo_text_extents(cr, txt, &extents);

			cairo_move_to(cr, x - extents.width / 2, h - y - 3);

			cairo_show_text(cr, txt);
			g_free(txt);
		}
	}

	cairo_destroy(cr);
}

static void zoomcallib_destroy(GtkObject* object)
{
	ZoomCallib* callib;
	ZoomCallibClass* klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(IS_ZOOM_CALLIB(object));

	callib = ZOOM_CALLIB(object);

	klass = (ZoomCallibClass*) gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy)
	{
		(*GTK_OBJECT_CLASS(klass)->destroy)(object);
	}
}
