#include "XournalWidget.h"

#include "control/Control.h"
#include "control/tools/EditSelection.h"
#include "control/settings/Settings.h"
#include "gui/Layout.h"
#include "gui/inputdevices/BaseInputDevice.h"
#include "gui/inputdevices/DirectAxisInputDevice.h"
#include "gui/inputdevices/NewGtkInputDevice.h"
#include "gui/pageposition/PagePositionCache.h"
#include "gui/pageposition/PagePositionHandler.h"
#include "gui/Shadow.h"
#include "gui/XournalView.h"
#include "util/DeviceListHelper.h"

#include <config-debug.h>
#include <Rectangle.h>
#include <Util.h>
#include <XInputUtils.h>

#include <gdk/gdk.h>

#include <gdk/gdkkeysyms.h>
#include <math.h>
#include <iostream>
using std::cout;
using std::endl;

static void gtk_xournal_class_init(GtkXournalClass* klass);
static void gtk_xournal_init(GtkXournal* xournal);
static void gtk_xournal_get_preferred_width(GtkWidget *widget,
                                            gint *minimal_width,
                                            gint *natural_width);
static void gtk_xournal_get_preferred_height(GtkWidget *widget,
                                             gint *minimal_height,
                                             gint *natural_height);
static void gtk_xournal_size_allocate(GtkWidget* widget,
                                      GtkAllocation* allocation);
static void gtk_xournal_realize(GtkWidget* widget);
static gboolean gtk_xournal_draw(GtkWidget* widget, cairo_t* cr);
static void gtk_xournal_destroy(GtkWidget* object);

static gboolean gtk_xournal_key_press_event(GtkWidget* widget, GdkEventKey* event);
static gboolean gtk_xournal_key_release_event(GtkWidget* widget, GdkEventKey* event);

GType gtk_xournal_get_type(void)
{
	static GType gtk_xournal_type = 0;

	if(!gtk_xournal_type)
	{
		static const GTypeInfo gtk_xournal_info =
		{
			sizeof(GtkXournalClass),
			// base initialize
			NULL,
			// base finalize
			NULL,
			// class initialize
			(GClassInitFunc) gtk_xournal_class_init,
			// class finalize
			NULL,
			// class data,
			NULL,
			// instance size
			sizeof(GtkXournal),
			// n_preallocs
			0,
			// instance init
			(GInstanceInitFunc) gtk_xournal_init,
			// value table
			(const GTypeValueTable*) NULL
		};

		gtk_xournal_type = g_type_register_static(GTK_TYPE_WIDGET,
		                                          "GtkXournal",
		                                          &gtk_xournal_info,
		                                          (GTypeFlags) 0);
	}

	return gtk_xournal_type;
}

GtkWidget* gtk_xournal_new(XournalView* view, GtkScrollable* parent)
{
	GtkXournal* xoj = GTK_XOURNAL(g_object_new(gtk_xournal_get_type(), NULL));
	xoj->view = view;
	xoj->scrollX = 0;
	xoj->scrollY = 0;
	xoj->x = 0;
	xoj->y = 0;
	xoj->layout = new Layout(view,
	                         gtk_scrollable_get_hadjustment(parent),
	                         gtk_scrollable_get_vadjustment(parent));
	xoj->currentInputPage = NULL;
	xoj->pagePositionCache = new PagePositionCache();

	xoj->lastMousePositionX = 0;
	xoj->lastMousePositionY = 0;
	xoj->scrollOffsetX = 0;
	xoj->scrollOffsetY = 0;
	xoj->inScrolling = false;

	xoj->selection = NULL;
	xoj->shiftDown = false;

	Settings* settings = view->getControl()->getSettings();

	SElement& inputSettings = settings->getCustomElement("inputHandling");

	string selectedInputType;
	inputSettings.getString("type", selectedInputType);

	if (selectedInputType == "01-gtk")
	{
		xoj->input = new BaseInputDevice(GTK_WIDGET(xoj), view);
	}
	else if (selectedInputType == "02-direct")
	{
		xoj->input = new DirectAxisInputDevice(GTK_WIDGET(xoj), view);
	}
	else if (selectedInputType == "03-gtk")
	{
		xoj->input = new NewGtkInputDevice(GTK_WIDGET(xoj), view);
	}
	else // selectedInputType == "auto"
	{
		// No autodection performed yet, need some infos from users
		xoj->input = new DirectAxisInputDevice(GTK_WIDGET(xoj), view);
	}

	xoj->input->initWidget();

	return GTK_WIDGET(xoj);
}

static void gtk_xournal_class_init(GtkXournalClass* klass)
{
	GtkWidgetClass* widget_class;

	widget_class = (GtkWidgetClass*) klass;

	widget_class->realize = gtk_xournal_realize;
	widget_class->get_preferred_width = gtk_xournal_get_preferred_width;
	widget_class->get_preferred_height = gtk_xournal_get_preferred_height;
	widget_class->size_allocate = gtk_xournal_size_allocate;

	widget_class->key_press_event = gtk_xournal_key_press_event;
	widget_class->key_release_event = gtk_xournal_key_release_event;

	widget_class->draw = gtk_xournal_draw;

	widget_class->destroy = gtk_xournal_destroy;
}

static gboolean gtk_xournal_key_press_event(GtkWidget* widget, GdkEventKey* event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	// Shift alone pressed. Is there a constant?
	if (event->is_modifier && (event->keyval == 0xFFE1))
	{
		xournal->shiftDown = true;
	}

	EditSelection* selection = xournal->selection;
	if (selection)
	{
		int d = 3;

		if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_SHIFT_MASK))
		{
			if (event->state & GDK_MOD1_MASK)
			{
				d = 1;
			}
			else
			{
				d = 20;
			}
		}

		if (event->keyval == GDK_KEY_Left)
		{
			selection->moveSelection(d, 0);
			return true;
		}
		else if (event->keyval == GDK_KEY_Up)
		{
			selection->moveSelection(0, d);
			return true;
		}
		else if (event->keyval == GDK_KEY_Right)
		{
			selection->moveSelection(-d, 0);
			return true;
		}
		else if (event->keyval == GDK_KEY_Down)
		{
			selection->moveSelection(0, -d);
			return true;
		}
	}

	return xournal->view->onKeyPressEvent(event);
}

static gboolean gtk_xournal_key_release_event(GtkWidget* widget, GdkEventKey* event)
{
	g_return_val_if_fail(widget != NULL, false);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), false);
	g_return_val_if_fail(event != NULL, false);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	if (event->is_modifier && (event->state & GDK_SHIFT_MASK))
	{
		xournal->shiftDown = false;
	}

	return xournal->view->onKeyReleaseEvent(event);
}

Rectangle* gtk_xournal_get_visible_area(GtkWidget* widget, XojPageView* p)
{
	g_return_val_if_fail(widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), NULL);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	GtkAllocation allocation = { 0 };
	gtk_widget_get_allocation(widget, &allocation);
	int viewHeight = allocation.height;
	int viewWidth = allocation.width;

	GdkRectangle r1;
	GdkRectangle r2;
	GdkRectangle r3 = { 0, 0, 0, 0 };

	r1.x = p->getX();
	r1.y = p->getY();
	r1.width = p->getDisplayWidth();
	r1.height = p->getDisplayHeight();

	r2.x = xournal->x;
	r2.y = xournal->y;
	r2.width = viewWidth;
	r2.height = viewHeight;

	gdk_rectangle_intersect(&r1, &r2, &r3);

	if (r3.width == 0 && r3.height == 0)
	{
		return NULL;
	}

	double zoom = xournal->view->getZoom();

	return new Rectangle(MAX(r3.x, 0) / zoom, MAX(r3.y, 0) / zoom, r3.width / zoom, r3.height / zoom);
}

Layout* gtk_xournal_get_layout(GtkWidget* widget)
{
	g_return_val_if_fail(widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), NULL);

	GtkXournal* xournal = GTK_XOURNAL(widget);
	return xournal->layout;
}

static void gtk_xournal_init(GtkXournal* xournal)
{
	GtkWidget* widget = GTK_WIDGET(xournal);

	gtk_widget_set_can_focus(widget, TRUE);
}

static void
gtk_xournal_get_preferred_width(GtkWidget *widget,
                                gint      *minimal_width,
                                gint      *natural_width)
{
	GtkXournal* xournal = GTK_XOURNAL(widget);

	*minimal_width = *natural_width = xournal->layout->getLayoutWidth();
}

static void
gtk_xournal_get_preferred_height(GtkWidget *widget,
                                 gint      *minimal_height,
                                 gint      *natural_height)
{
	GtkXournal* xournal = GTK_XOURNAL(widget);

	*minimal_height = *natural_height = xournal->layout->getLayoutHeight();
}

static void gtk_xournal_size_allocate(GtkWidget* widget, GtkAllocation* allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));
	g_return_if_fail(allocation != NULL);

	gtk_widget_set_allocation(widget, allocation);

	if (gtk_widget_get_realized(widget))
	{
		gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y, allocation->width, allocation->height);
	}

	GtkXournal* xournal = GTK_XOURNAL(widget);

	xournal->layout->setSize(allocation->width, allocation->height);
}

static void gtk_xournal_realize(GtkWidget* widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));

	gtk_widget_set_realized(widget, TRUE);

	gtk_widget_set_hexpand(widget, TRUE);
	gtk_widget_set_vexpand(widget, TRUE);

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = allocation.x;
	attributes.y = allocation.y;
	attributes.width = allocation.width;
	attributes.height = allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask));
	gdk_window_set_user_data(gtk_widget_get_window(widget), widget);
}

static void gtk_xournal_draw_shadow(GtkXournal* xournal, cairo_t* cr, int left,
                                    int top, int width, int height, bool selected)
{
	if (selected)
	{
		Shadow::drawShadow(cr, left - 2, top - 2, width + 4, height + 4);

		Settings* settings = xournal->view->getControl()->getSettings();

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
	}
	else
	{
		Shadow::drawShadow(cr, left, top, width, height);
	}
}

void gtk_xournal_repaint_area(GtkWidget* widget, int x1, int y1, int x2,
                              int y2)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));

	GtkXournal* xournal = GTK_XOURNAL(widget);

	x1 -= xournal->x;
	x2 -= xournal->x;
	y1 -= xournal->y;
	y2 -= xournal->y;

	if (x2 < 0 || y2 < 0)
	{
		return; // outside visible area
	}

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(widget, &alloc);

	if (x1 > alloc.width || y1 > alloc.height)
	{
		return; // outside visible area
	}

	gtk_widget_queue_draw_area(widget, x1, y1, x2 - x1, y2 - y1);
}

static gboolean gtk_xournal_draw(GtkWidget* widget, cairo_t* cr)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), FALSE);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	ArrayIterator<XojPageView*> it = xournal->view->pageViewIterator();

	double x1, x2, y1, y2;

	cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

	// Draw background
	Settings* settings = xournal->view->getControl()->getSettings();
	Util::cairo_set_source_rgbi(cr, settings->getBackgroundColor());
	cairo_rectangle(cr, x1, y1, x2 - x1, y2 - y1);
	cairo_fill(cr);

	Rectangle clippingRect(x1 - 10, y1 - 10, x2 - x1 + 20, y2 - y1 + 20);

	while (it.hasNext())
	{
		XojPageView* pv = it.next();

		int px = pv->getX();
		int py = pv->getY();
		int pw = pv->getDisplayWidth();
		int ph = pv->getDisplayHeight();

		if (!clippingRect.intersects(pv->getRect()))
		{
			continue;
		}

		gtk_xournal_draw_shadow(xournal, cr, px, py, pw, ph, pv->isSelected());

		cairo_save(cr);
		cairo_translate(cr, px, py);

		pv->paintPage(cr, NULL);
		cairo_restore(cr);
	}

	if (xournal->selection)
	{
		double zoom = xournal->view->getZoom();

		Redrawable* red = xournal->selection->getView();
		cairo_translate(cr, red->getX(), red->getY());

		xournal->selection->paint(cr, zoom);
	}

	return TRUE;
}

static void gtk_xournal_destroy(GtkWidget* object)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(object));

	GtkXournal* xournal = GTK_XOURNAL(object);
	delete xournal->pagePositionCache;
	xournal->pagePositionCache = NULL;

	delete xournal->selection;
	xournal->selection = NULL;

	delete xournal->layout;
	xournal->layout = NULL;

	delete xournal->input;
	xournal->input = NULL;
}

