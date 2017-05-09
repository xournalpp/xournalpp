#include "XournalWidget.h"
#include "../XournalView.h"
#include <Util.h>
#include <Rectangle.h>
#include "../Shadow.h"
#include "../../control/Control.h"
#include "../../control/settings/Settings.h"
#include "../../cfg.h"
#include "../pageposition/PagePositionCache.h"
#include "../pageposition/PagePositionHandler.h"
#include "../Cursor.h"
#include "../../control/tools/EditSelection.h"
#include "../../control/settings/ButtonConfig.h"
#include "../Layout.h"

#include <gdk/gdkkeysyms.h>
#include <math.h>

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
static gboolean gtk_xournal_button_press_event(GtkWidget* widget,
                                               GdkEventButton* event);
static gboolean gtk_xournal_button_release_event(GtkWidget* widget,
                                                 GdkEventButton* event);
static gboolean gtk_xournal_motion_notify_event(GtkWidget* widget,
                                                GdkEventMotion* event);
static gboolean gtk_xournal_key_press_event(GtkWidget* widget,
                                            GdkEventKey* event);
static gboolean gtk_xournal_key_release_event(GtkWidget* widget,
                                              GdkEventKey* event);
static void gtk_xournal_scroll_mouse_event(GtkXournal* xournal,
                                           GdkEventMotion* event);

PageView *current_view;

GType gtk_xournal_get_type(void)
{
	static GType gtk_xournal_type = 0;

	/*
	 * TODO: check if this is correct
	 */
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
	widget_class->button_press_event = gtk_xournal_button_press_event;
	widget_class->button_release_event = gtk_xournal_button_release_event;
	widget_class->motion_notify_event = gtk_xournal_motion_notify_event;

	widget_class->key_press_event = gtk_xournal_key_press_event;
	widget_class->key_release_event = gtk_xournal_key_release_event;

	widget_class->draw = gtk_xournal_draw;

	widget_class->destroy = gtk_xournal_destroy;
}

static gboolean gtk_xournal_key_press_event(GtkWidget* widget,
                                            GdkEventKey* event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	EditSelection* selection = xournal->selection;
	if (selection)
	{
		int d = 3;

		if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_SHIFT_MASK))
		{
			if ( event->state & GDK_MOD1_MASK )
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
			return TRUE;
		}
		else if (event->keyval == GDK_KEY_Up)
		{
			selection->moveSelection(0, d);
			return TRUE;
		}
		else if (event->keyval == GDK_KEY_Right)
		{
			selection->moveSelection(-d, 0);
			return TRUE;
		}
		else if (event->keyval == GDK_KEY_Down)
		{
			selection->moveSelection(0, -d);
			return TRUE;
		}
	}

	return xournal->view->onKeyPressEvent(event);
}

static gboolean gtk_xournal_key_release_event(GtkWidget* widget,
                                              GdkEventKey* event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	return xournal->view->onKeyReleaseEvent(event);
}

Rectangle* gtk_xournal_get_visible_area(GtkWidget* widget, PageView* p)
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

	return new Rectangle(MAX(r3.x, 0) / zoom, MAX(r3.y, 0) / zoom, r3.width / zoom,
	                     r3.height / zoom);
}

bool gtk_xournal_scroll_callback(GtkXournal* xournal)
{
	gdk_threads_enter();

	xournal->layout->scrollRelativ(xournal->scrollOffsetX, xournal->scrollOffsetY);

	// Scrolling done, so reset our counters
	xournal->scrollOffsetX = 0;
	xournal->scrollOffsetY = 0;

	gdk_threads_leave();

	return false;
}

static void gtk_xournal_scroll_mouse_event(GtkXournal* xournal,
                                           GdkEventMotion* event)
{
  // use root coordinates as reference point because
  //scrolling changes window relative coordinates
  //see github Gnome/evince@1adce5486b10e763bed869
  int x_root = event->x_root;
  int y_root = event->y_root;

	if (xournal->lastMousePositionX - x_root == 0 &&
	    xournal->lastMousePositionY - y_root == 0)
	{
		return;
	}

	if (xournal->scrollOffsetX == 0 && xournal->scrollOffsetY == 0)
	{
    xournal->scrollOffsetX = xournal->lastMousePositionX - x_root;
    xournal->scrollOffsetY = xournal->lastMousePositionY - y_root;

		g_idle_add((GSourceFunc) gtk_xournal_scroll_callback, xournal);
		//gtk_xournal_scroll_callback(xournal);
    xournal->lastMousePositionX = x_root;
		xournal->lastMousePositionY = y_root;
	}
}

PageView* gtk_xournal_get_page_view_for_pos_cached(GtkXournal* xournal, int x,
                                                   int y)
{
	x += xournal->x;
	y += xournal->y;

	PagePositionHandler* pph = xournal->view->getPagePositionHandler();

	return pph->getViewAt(x, y, xournal->pagePositionCache);
}

Layout* gtk_xournal_get_layout(GtkWidget* widget)
{
	g_return_val_if_fail(widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), NULL);

	GtkXournal* xournal = GTK_XOURNAL(widget);
	return xournal->layout;
}

static bool change_tool(Settings* settings, GdkEventButton* event,
                        GtkXournal* xournal)
{
	ButtonConfig* cfg = NULL;
	ButtonConfig* cfgTouch = settings->getTouchButtonConfig();
	ToolHandler* h = xournal->view->getControl()->getToolHandler();

	GdkEvent* rawEvent = (GdkEvent*) event;
	GdkDevice* device = gdk_event_get_source_device(rawEvent);

	if (gdk_device_get_source(device) == GDK_SOURCE_PEN)
	{
		if (event->button == 2)
		{
			cfg = settings->getStylusButtonConfig();
		}
		else if (event->button == 3)
		{
			cfg = settings->getStylus2ButtonConfig();
		}
	}
	 else if (event->button == 2)   // Middle Button
	{
		cfg = settings->getMiddleButtonConfig();
	}
	else if (event->button == 3 && !xournal->selection)     // Right Button
	{
		cfg = settings->getRightButtonConfig();
	}
	else if (gdk_device_get_source(device) == GDK_SOURCE_ERASER)
	{
		cfg = settings->getEraserButtonConfig();
	}
	else if (cfgTouch->device == gdk_device_get_name(device))
	{
		cfg = cfgTouch;

		// If an action is defined we do it, even if it's a drawing action...
		if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE)
		{
			ToolType tool = h->getToolType();
			if (tool == TOOL_PEN || tool == TOOL_ERASER || tool == TOOL_HILIGHTER)
			{
				printf("ignore touchscreen for drawing!\n");
				return true;
			}
		}
	}

	if (cfg && cfg->getAction() != TOOL_NONE)
	{
		h->copyCurrentConfig();
		cfg->acceptActions(h);
	}

	return false;
}

gboolean gtk_xournal_button_press_event(GtkWidget* widget,
                                        GdkEventButton* event)
{
	GtkXournal* xournal = GTK_XOURNAL(widget);
	Settings* settings = xournal->view->getControl()->getSettings();
   //gtk_gesture_is_recognized is always false (bug, programming error?)
   //workaround with additional variable zoom_gesture_active 
   if (xournal->view->zoom_gesture_active)
	  {
      return TRUE;
    }
	if (event->type != GDK_BUTTON_PRESS)
	{
		return FALSE; // this event is not handled here
	}

	if (event->button > 3)   // scroll wheel events
	{
		return FALSE;
	}

	gtk_widget_grab_focus(widget);

	// none button release event was sent, send one now
	if (xournal->currentInputPage)
	{
		GdkEventButton ev = *event;
		xournal->currentInputPage->translateEvent((GdkEvent*) &ev, xournal->x,
		                                          xournal->y);
		xournal->currentInputPage->onButtonReleaseEvent(widget, &ev);
	}

	ToolHandler* h = xournal->view->getControl()->getToolHandler();

	// Change the tool depending on the key or device
	if(change_tool(settings, event, xournal))
		return TRUE;

	// hand tool don't change the selection, so you can scroll e.g.
	// with your touchscreen without remove the selection
	if (h->getToolType() == TOOL_HAND)
	{
		Cursor* cursor = xournal->view->getCursor();
		cursor->setMouseDown(true);
		xournal->inScrolling = true;
    //set reference
    xournal->lastMousePositionX=event->x_root;
		xournal->lastMousePositionY=event->y_root;

		return TRUE;
	}
	else if (xournal->selection)
	{
		EditSelection* selection = xournal->selection;

		PageView* view = selection->getView();
		GdkEventButton ev = *event;
		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);
		CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y,
		                                                                xournal->view->getZoom());
		if (selType)
		{

			if(selType == CURSOR_SELECTION_MOVE && event->button == 3)
			{
				selection->copySelection();
			}

			xournal->view->getCursor()->setMouseDown(true);
			xournal->selection->mouseDown(selType, ev.x, ev.y);
			return TRUE;
		}
		else
		{
			xournal->view->clearSelection();
			if(change_tool(settings, event, xournal))
				return TRUE;
		}
	}

	PageView* pv = gtk_xournal_get_page_view_for_pos_cached(xournal, event->x,
	                                                        event->y);

	current_view = pv;

	if (pv)
	{
		xournal->currentInputPage = pv;
		pv->translateEvent((GdkEvent*) event, xournal->x, xournal->y);

		xournal->view->getDocument()->indexOf(pv->getPage());
		return pv->onButtonPressEvent(widget, event);
	}

	return FALSE; // not handled
}

gboolean gtk_xournal_button_release_event(GtkWidget* widget,
                                          GdkEventButton* event)
{
	if (event->button > 3)   // scroll wheel events
	{
		return TRUE;
	}

	current_view = NULL;

	GtkXournal* xournal = GTK_XOURNAL(widget);

	Cursor* cursor = xournal->view->getCursor();
	ToolHandler* h = xournal->view->getControl()->getToolHandler();
  if (xournal->view->zoom_gesture_active)
    {
      return TRUE;
    }

	cursor->setMouseDown(false);

	xournal->inScrolling = false;

	EditSelection* sel = xournal->view->getSelection();
	if (sel)
	{
		sel->mouseUp();
	}

	bool res = false;
	if (xournal->currentInputPage)
	{
		xournal->currentInputPage->translateEvent((GdkEvent*) event, xournal->x,
		                                          xournal->y);
		res = xournal->currentInputPage->onButtonReleaseEvent(widget, event);
		xournal->currentInputPage = NULL;
	}

	EditSelection* tmpSelection = xournal->selection;
	xournal->selection = NULL;

	h->restoreLastConfig();

	// we need this workaround so it's possible to select something with the middle button
	if (tmpSelection)
	{
		xournal->view->setSelection(tmpSelection);
	}

	return res;
}

gboolean gtk_xournal_motion_notify_event(GtkWidget* widget,
                                         GdkEventMotion* event)
{
	GtkXournal* xournal = GTK_XOURNAL(widget);
	ToolHandler* h = xournal->view->getControl()->getToolHandler();

  if (xournal->view->zoom_gesture_active)
    {
      return TRUE;
    }


	if (h->getToolType() == TOOL_HAND)
	{
		if (xournal->inScrolling)
		{
			gtk_xournal_scroll_mouse_event(xournal, event);
			return TRUE;
		}
		return FALSE;
	}
	else if (xournal->selection)
	{
		EditSelection* selection = xournal->selection;

		PageView* view = selection->getView();
		GdkEventMotion ev = *event;
		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);

		if (xournal->selection->isMoving())
		{
			selection->mouseMove(ev.x, ev.y);
		}
		else
		{
			CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y,
			                                                                xournal->view->getZoom());
			xournal->view->getCursor()->setMouseSelectionType(selType);
		}
		return TRUE;
	}

	PageView* pv = NULL;

	if(current_view)
	{
		pv = current_view;
	}
	else
	{
		pv = gtk_xournal_get_page_view_for_pos_cached(xournal,
	                                                event->x,
	                                                event->y);
	}

	xournal->view->getCursor()->setInsidePage(pv != NULL);

	if (pv)
	{
		// allow events only to a single page!
		if (xournal->currentInputPage == NULL || pv == xournal->currentInputPage)
		{
			pv->translateEvent((GdkEvent*) event, xournal->x, xournal->y);
			return pv->onMotionNotifyEvent(widget, event);;
		}
	}

	return FALSE;
}

static void gtk_xournal_init(GtkXournal* xournal)
{
	GtkWidget* widget = GTK_WIDGET(xournal);

	gtk_widget_set_can_focus(widget, TRUE);

	int events = GDK_EXPOSURE_MASK;
	events |= GDK_POINTER_MOTION_MASK;
	events |= GDK_EXPOSURE_MASK;
	events |= GDK_BUTTON_MOTION_MASK;
	//not sure if GDK_TOUCH_MASK is needed
  events |= GDK_TOUCH_MASK;
	events |= GDK_BUTTON_PRESS_MASK;
	events |= GDK_BUTTON_RELEASE_MASK;
	events |= GDK_ENTER_NOTIFY_MASK;
	events |= GDK_LEAVE_NOTIFY_MASK;
	events |= GDK_KEY_PRESS_MASK;
	events |= GDK_SCROLL_MASK;

	gtk_widget_set_events(widget, events);
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

static void gtk_xournal_size_allocate(GtkWidget* widget,
                                      GtkAllocation* allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));
	g_return_if_fail(allocation != NULL);

	gtk_widget_set_allocation(widget, allocation);

	if (gtk_widget_get_realized(widget))
	{
		gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y,
		                       allocation->width, allocation->height);
	}

	GtkXournal* xournal = GTK_XOURNAL(widget);

	xournal->layout->setSize(allocation->width, allocation->height);
}

static void gtk_xournal_realize(GtkWidget* widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;
	GtkAllocation allocation;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));

	gtk_widget_set_realized(widget, TRUE);

	gtk_widget_set_hexpand(widget, TRUE);
	gtk_widget_set_vexpand(widget, TRUE);

	gtk_widget_get_allocation(widget, &allocation);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = allocation.x;
	attributes.y = allocation.y;
	attributes.width = allocation.width;
	attributes.height = allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	gtk_widget_set_window(widget,
	                      gdk_window_new(gtk_widget_get_parent_window(widget),
	                                     &attributes, attributes_mask));

	gtk_widget_modify_bg(widget, GTK_STATE_NORMAL,
	                     &gtk_widget_get_style(widget)->dark[GTK_STATE_NORMAL]);

	gdk_window_set_user_data(gtk_widget_get_window(widget), widget);

	gtk_widget_style_attach(widget);
	gtk_style_set_background(gtk_widget_get_style(widget),
	                         gtk_widget_get_window(widget),
	                         GTK_STATE_NORMAL);
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

	ArrayIterator<PageView*> it = xournal->view->pageViewIterator();

	double x1, x2, y1, y2;

	cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

	Rectangle clippingRect(x1 - 10,
	                       y1 - 10,
	                       x2 - x1 + 20,
	                       y2 - y1 + 20);

	while (it.hasNext())
	{
		PageView* pv = it.next();

		int px = pv->getX();
		int py = pv->getY();
		int pw = pv->getDisplayWidth();
		int ph = pv->getDisplayHeight();

		if(!clippingRect.intersects(pv->getRect()))
		{
			continue;
		}

		gtk_xournal_draw_shadow(xournal, cr,
		                        px, py, pw, ph,
		                        pv->isSelected());

		cairo_save(cr);
		cairo_translate(cr, px, py);

		pv->paintPage(cr, NULL);
		cairo_restore(cr);
	}

	if(xournal->selection)
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

	/*
	 * TODO: Do we need this?
	GtkXournalClass* klass = (GtkXournalClass*) gtk_type_class(
	                             gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy)
	{
		(*GTK_OBJECT_CLASS(klass)->destroy)(object);
	}
	*/
}

