#include "XournalWidget.h"
#include "../XournalView.h"
#include "../../util/Util.h"
#include "../Shadow.h"
#include "../../control/Control.h"
#include "../../control/settings/Settings.h"

static void gtk_xournal_class_init(GtkXournalClass * klass);
static void gtk_xournal_init(GtkXournal * xournal);
static void gtk_xournal_size_request(GtkWidget * widget, GtkRequisition * requisition);
static void gtk_xournal_size_allocate(GtkWidget * widget, GtkAllocation * allocation);
static void gtk_xournal_realize(GtkWidget * widget);
static gboolean gtk_xournal_expose(GtkWidget * widget, GdkEventExpose * event);
static void gtk_xournal_destroy(GtkObject * object);

GtkType gtk_xournal_get_type(void) {
	static GtkType gtk_xournal_type = 0;

	if (!gtk_xournal_type) {
		static const GtkTypeInfo gtk_xournal_info = { "GtkXournal", sizeof(GtkXournal), sizeof(GtkXournalClass), (GtkClassInitFunc) gtk_xournal_class_init,
				(GtkObjectInitFunc) gtk_xournal_init, NULL, NULL, (GtkClassInitFunc) NULL };
		gtk_xournal_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_xournal_info);
	}

	return gtk_xournal_type;
}

GtkWidget * gtk_xournal_new(XournalView * view) {
	GtkXournal * xoj = GTK_XOURNAL(gtk_type_new(gtk_xournal_get_type()));
	xoj->view = view;
	xoj->scrollX = 0;
	xoj->scrollY = 0;
	return GTK_WIDGET(xoj);
}

static void gtk_xournal_class_init(GtkXournalClass *klass) {
	GtkWidgetClass * widget_class;
	GtkObjectClass * object_class;

	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = gtk_xournal_realize;
	widget_class->size_request = gtk_xournal_size_request;
	widget_class->size_allocate = gtk_xournal_size_allocate;

	widget_class->expose_event = gtk_xournal_expose;

	object_class->destroy = gtk_xournal_destroy;
}

static void gtk_xournal_init(GtkXournal * xournal) {
	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(xournal), GTK_CAN_FOCUS);
}

// gtk_widget_get_preferred_size()
// the output don't interesset anybody...
static void gtk_xournal_size_request(GtkWidget * widget, GtkRequisition * requisition) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 100;
	requisition->height = 100;
}

static void gtk_xournal_size_allocate(GtkWidget * widget, GtkAllocation * allocation) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize(widget->window, allocation->x, allocation->y, allocation->width, allocation->height);
	}
}

static void gtk_xournal_realize(GtkWidget * widget) {
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 80;
	attributes.height = 100;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);
	gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &widget->style->dark[GTK_STATE_NORMAL]);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

void gtk_xournal_redraw(GtkWidget * widget) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));

	GtkXournal * xournal = GTK_XOURNAL(widget);

	GdkRegion * region = gdk_drawable_get_clip_region(GTK_WIDGET(xournal)->window);
	gdk_window_invalidate_region(GTK_WIDGET(xournal)->window, region, TRUE);
	gdk_window_process_updates(GTK_WIDGET(xournal)->window, TRUE);
}


/**
 * Change event handling between XInput and Core
 */
void gtk_xournal_update_xevent(GtkWidget * widget) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));

	GtkXournal * xournal = GTK_XOURNAL(widget);

	Settings * settings = xournal->view->getControl()->getSettings();

	if (!gtk_check_version(2, 17, 0)) {
		/* GTK+ 2.17 and later: everybody shares a single native window,
		 so we'll never get any core events, and we might as well set
		 extension events the way we're supposed to. Doing so helps solve
		 crasher bugs in 2.17, and prevents us from losing two-button
		 events in 2.18 */
		gtk_widget_set_extension_events(widget, settings->isUseXInput() ? GDK_EXTENSION_EVENTS_ALL : GDK_EXTENSION_EVENTS_NONE);
	} else {
		/* GTK+ 2.16 and earlier: we only activate extension events on the
		 PageViews's parent GdkWindow. This allows us to keep receiving core
		 events. */
		gdk_input_set_extension_events(widget->window, GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
				settings->isUseXInput() ? GDK_EXTENSION_EVENTS_ALL : GDK_EXTENSION_EVENTS_NONE);
	}

}

static void gtk_xournal_draw_shadow(GtkXournal * xournal, cairo_t * cr, int top, int left, int width, int height, bool selected) {
	if (selected) {
		Shadow::drawShadow(cr, left - 2, top - 2, width + 4, height + 4);

		Settings * settings = xournal->view->getControl()->getSettings();

		// Draw border
		Util::cairo_set_source_rgbi(cr, settings->getSelectionColor());
		cairo_set_line_width(cr, 4.0);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

		cairo_move_to(cr, left, top);
		cairo_line_to(cr, left, top + height);
		cairo_line_to(cr, left + width, top + height);
		cairo_line_to(cr, left + width, top);
		cairo_close_path(cr);

		cairo_stroke(cr);
	} else {
		Shadow::drawShadow(cr, left, top, width, height);
	}
}

static gboolean gtk_xournal_expose(GtkWidget * widget, GdkEventExpose * event) {
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	GtkXournal * xournal = GTK_XOURNAL(widget);

	cairo_t * cr = gdk_cairo_create(GTK_WIDGET(widget)->window);

	ArrayIterator<PageView *> it = xournal->view->pageViewIterator();


	while(it.hasNext()) {
		PageView * pv = it.next();


//		gtk_xournal_draw_shadow(xournal, cr, 10+i*100, 20, 50, 50, i < 3);
	}

	cairo_destroy(cr);

	// dont call any event handler, all is draw here
	return true;
}

static void gtk_xournal_destroy(GtkObject * object) {
	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(object));

	GtkXournal * xournal = GTK_XOURNAL(object);

	GtkXournalClass * klass = (GtkXournalClass *) gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(*GTK_OBJECT_CLASS(klass)->destroy)(object);
	}
}
