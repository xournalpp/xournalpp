#include "XournalWidget.h"
#include "../XournalView.h"
#include <Util.h>
#include <Rectangle.h>
#include "../Shadow.h"
#include "../../control/Control.h"
#include "../../control/settings/Settings.h"
#include <XInputUtils.h>
#include "../../cfg.h"
#include "../pageposition/PagePositionCache.h"
#include "../pageposition/PagePositionHandler.h"
#include "../Cursor.h"
#include "../../control/tools/EditSelection.h"
#include "../../control/settings/ButtonConfig.h"
#include "Scrollbar.h"
#include "../Layout.h"

#include <gdk/gdkkeysyms.h>
#include <math.h>

static void gtk_xournal_class_init(GtkXournalClass * klass);
static void gtk_xournal_init(GtkXournal * xournal);
static void gtk_xournal_size_request(GtkWidget * widget, GtkRequisition * requisition);
static void gtk_xournal_size_allocate(GtkWidget * widget, GtkAllocation * allocation);
static void gtk_xournal_realize(GtkWidget * widget);
static gboolean gtk_xournal_expose(GtkWidget * widget, GdkEventExpose * event);
static void gtk_xournal_destroy(GtkObject * object);
static void gtk_xournal_connect_scrollbars(GtkXournal * xournal);
static gboolean gtk_xournal_button_press_event(GtkWidget * widget, GdkEventButton * event);
static gboolean gtk_xournal_button_release_event(GtkWidget * widget, GdkEventButton * event);
static gboolean gtk_xournal_motion_notify_event(GtkWidget * widget, GdkEventMotion * event);
static void gtk_xournal_set_adjustment_upper(GtkAdjustment *adj, gdouble upper, gboolean always_emit_changed);
static gboolean gtk_xournal_key_press_event(GtkWidget * widget, GdkEventKey * event);
static gboolean gtk_xournal_key_release_event(GtkWidget * widget, GdkEventKey * event);
gboolean gtk_xournal_scroll_event(GtkWidget * widget, GdkEventScroll * event);
static void gtk_xournal_scroll_mouse_event(GtkXournal * xournal, GdkEventMotion * event);

GtkType gtk_xournal_get_type(void) {
	static GtkType gtk_xournal_type = 0;

	if (!gtk_xournal_type) {
		static const GtkTypeInfo gtk_xournal_info = {
				"GtkXournal",
				sizeof(GtkXournal),
				sizeof(GtkXournalClass),
				(GtkClassInitFunc) gtk_xournal_class_init,
				(GtkObjectInitFunc) gtk_xournal_init,
				NULL,
				NULL,
				(GtkClassInitFunc) NULL
		};
		gtk_xournal_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_xournal_info);
	}

	return gtk_xournal_type;
}

GtkWidget * gtk_xournal_new(XournalView * view) {
	GtkXournal * xoj = GTK_XOURNAL(gtk_type_new(gtk_xournal_get_type()));
	xoj->view = view;
	xoj->scrollX = 0;
	xoj->scrollY = 0;
	xoj->x = 0;
	xoj->y = 0;
	xoj->layout = new Layout(view);
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

static void gtk_xournal_class_init(GtkXournalClass * klass) {
	GtkWidgetClass * widget_class;
	GtkObjectClass * object_class;

	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = gtk_xournal_realize;
	widget_class->size_request = gtk_xournal_size_request;
	widget_class->size_allocate = gtk_xournal_size_allocate;
	widget_class->button_press_event = gtk_xournal_button_press_event;
	widget_class->button_release_event = gtk_xournal_button_release_event;
	widget_class->motion_notify_event = gtk_xournal_motion_notify_event;
	widget_class->enter_notify_event = XInputUtils::onMouseEnterNotifyEvent;
	widget_class->leave_notify_event = XInputUtils::onMouseLeaveNotifyEvent;
	widget_class->scroll_event = gtk_xournal_scroll_event;

	widget_class->key_press_event = gtk_xournal_key_press_event;
	widget_class->key_release_event = gtk_xournal_key_release_event;

	widget_class->expose_event = gtk_xournal_expose;

	object_class->destroy = gtk_xournal_destroy;
}

static gboolean gtk_xournal_key_press_event(GtkWidget * widget, GdkEventKey * event) {
	g_return_val_if_fail(widget != NULL, false);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), false);
	g_return_val_if_fail(event != NULL, false);

	GtkXournal * xournal = GTK_XOURNAL(widget);

	EditSelection * selection = xournal->selection;
	if (selection) {
		int d = 10;

		if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_SHIFT_MASK)) {
			d = 1;
		}

		if (event->keyval == GDK_Left) {
			selection->moveSelection(-d, 0);
			return true;
		} else if (event->keyval == GDK_Up) {
			selection->moveSelection(0, -d);
			return true;
		} else if (event->keyval == GDK_Right) {
			selection->moveSelection(d, 0);
			return true;
		} else if (event->keyval == GDK_Down) {
			selection->moveSelection(0, d);
			return true;
		}
	}

	return xournal->view->onKeyPressEvent(event);
}

static gboolean gtk_xournal_key_release_event(GtkWidget * widget, GdkEventKey * event) {
	g_return_val_if_fail(widget != NULL, false);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), false);
	g_return_val_if_fail(event != NULL, false);

	GtkXournal * xournal = GTK_XOURNAL(widget);

	return xournal->view->onKeyReleaseEvent(event);
}

gboolean gtk_xournal_scroll_event(GtkWidget * widget, GdkEventScroll * event) {
#ifdef INPUT_DEBUG
	// true: Core event, false: XInput event
	gboolean isCore = (event->device == gdk_device_get_core_pointer());

	INPUTDBG("Scroll (%s) (x,y)=(%.2f,%.2f), direction %d, modifier %x, isCore %i", gdk_device_get_name(event->device), event->x, event->y,
			event->direction, event->state, isCore);
#endif

	g_return_val_if_fail(GTK_XOURNAL(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	GtkXournal * xournal = GTK_XOURNAL(widget);
	return xournal->layout->scrollEvent(event);
}

Rectangle * gtk_xournal_get_visible_area(GtkWidget * widget, PageView * p) {
	g_return_val_if_fail(widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), NULL);

	GtkXournal * xournal = GTK_XOURNAL(widget);

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

	if (r3.width == 0 && r3.height == 0) {
		return NULL;
	}

	double zoom = xournal->view->getZoom();

	return new Rectangle(MAX(r3.x, 0) / zoom, MAX(r3.y, 0) / zoom, r3.width / zoom, r3.height / zoom);
}

bool gtk_xournal_scroll_callback(GtkXournal * xournal) {
	gdk_threads_enter();

	xournal->layout->scrollRelativ(xournal->scrollOffsetX, xournal->scrollOffsetY);

	// Scrolling done, so reset our counters
	xournal->scrollOffsetX = 0;
	xournal->scrollOffsetY = 0;

	gdk_threads_leave();

	return false;
}

static void gtk_xournal_scroll_mouse_event(GtkXournal * xournal, GdkEventMotion * event) {
	int x = event->x;
	int y = event->y;

	if (xournal->lastMousePositionX - x == 0 && xournal->lastMousePositionY - y == 0) {
		return;
	}

	if (xournal->scrollOffsetX == 0 && xournal->scrollOffsetY == 0) {
		xournal->scrollOffsetX = xournal->lastMousePositionX - x;
		xournal->scrollOffsetY = xournal->lastMousePositionY - y;
		g_idle_add((GSourceFunc) gtk_xournal_scroll_callback, xournal);

		xournal->lastMousePositionX = x;
		xournal->lastMousePositionY = y;
	}
}

PageView * gtk_xournal_get_page_view_for_pos_cached(GtkXournal * xournal, int x, int y) {
	x += xournal->x;
	y += xournal->y;

	PagePositionHandler * pph = xournal->view->getPagePositionHandler();

	return pph->getViewAt(x, y, xournal->pagePositionCache);
}

Layout * gtk_xournal_get_layout(GtkWidget * widget) {
	g_return_val_if_fail(widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), NULL);

	GtkXournal * xournal = GTK_XOURNAL(widget);
	return xournal->layout;
}

gboolean gtk_xournal_button_press_event(GtkWidget * widget, GdkEventButton * event) {
	/**
	 * true: Core event, false: XInput event
	 */
	gboolean isCore = (event->device == gdk_device_get_core_pointer());

	INPUTDBG("ButtonPress (%s) (x,y)=(%.2f,%.2f), button %d, modifier %x, isCore %i", gdk_device_get_name(event->device), event->x, event->y,
			event->button, event->state, isCore);

	GtkXournal * xournal = GTK_XOURNAL(widget);
	Settings * settings = xournal->view->getControl()->getSettings();

	if(isCore && settings->isXinputEnabled() && settings->isIgnoreCoreEvents()) {
		INPUTDBG2("gtk_xournal_button_press_event return false (ignore core)");
		return false;
	}

	XInputUtils::fixXInputCoords((GdkEvent*) event, widget);

	if (event->type != GDK_BUTTON_PRESS) {
		INPUTDBG2("gtk_xournal_button_press_event return false (event->type != GDK_BUTTON_PRESS)");
		return false; // this event is not handled here
	}

	if (event->button > 3) { // scroll wheel events
		XInputUtils::handleScrollEvent(event, widget);
		INPUTDBG2("gtk_xournal_button_press_event return true handled scroll event");
		return true;
	}

	gtk_widget_grab_focus(widget);

	ToolHandler * h = xournal->view->getControl()->getToolHandler();

	// none button release event was sent, send one now
	if (xournal->currentInputPage) {
		INPUTDBG2("gtk_xournal_button_press_event (xournal->currentInputPage != NULL)");

		GdkEventButton ev = *event;
		xournal->currentInputPage->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);
		xournal->currentInputPage->onButtonReleaseEvent(widget, &ev);
	}

	// Change the tool depending on the key or device
	ButtonConfig * cfg = NULL;
	ButtonConfig * cfgTouch = settings->getTouchButtonConfig();
	if (event->button == 2) { // Middle Button
		cfg = settings->getMiddleButtonConfig();
	} else if (event->button == 3) { // Right Button
		cfg = settings->getRightButtonConfig();
	} else if (event->device->source == GDK_SOURCE_ERASER) {
		cfg = settings->getEraserButtonConfig();
	} else if (cfgTouch->device == event->device->name) {
		cfg = cfgTouch;

		// If an action is defined we do it, even if it's a drawing action...
		if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE) {
			ToolType tool = h->getToolType();
			if (tool == TOOL_PEN || tool == TOOL_ERASER || tool == TOOL_HILIGHTER) {
				printf("ignore touchscreen for drawing!\n");
				return true;
			}
		}
	}

	if (cfg && cfg->getAction() != TOOL_NONE) {
		h->copyCurrentConfig();
		cfg->acceptActions(h);
	}

	// hand tool don't change the selection, so you can scroll e.g.
	// with your touchscreen without remove the selection
	if (h->getToolType() == TOOL_HAND) {
		Cursor * cursor = xournal->view->getCursor();
		cursor->setMouseDown(true);
		xournal->lastMousePositionX = 0;
		xournal->lastMousePositionY = 0;
		xournal->inScrolling = true;
		gtk_widget_get_pointer(widget, &xournal->lastMousePositionX, &xournal->lastMousePositionY);

		INPUTDBG2("gtk_xournal_button_press_event (h->getToolType() == TOOL_HAND) return true");
		return true;
	} else if (xournal->selection) {
		EditSelection * selection = xournal->selection;

		PageView * view = selection->getView();
		GdkEventButton ev = *event;
		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);
		CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y, xournal->view->getZoom());
		if (selType) {
			xournal->view->getCursor()->setMouseDown(true);
			xournal->selection->mouseDown(selType, ev.x, ev.y);
			INPUTDBG2("gtk_xournal_button_press_event (selection) return true");
			return true;
		} else {
			xournal->view->clearSelection();
		}
	}

	PageView * pv = gtk_xournal_get_page_view_for_pos_cached(xournal, event->x, event->y);
	if (pv) {
		xournal->currentInputPage = pv;
		pv->translateEvent((GdkEvent*) event, xournal->x, xournal->y);
		INPUTDBG2("gtk_xournal_button_press_event (pv->onButtonPressEvent) return");

		xournal->view->getDocument()->indexOf(pv->getPage());
		return pv->onButtonPressEvent(widget, event);
	}

	INPUTDBG2("gtk_xournal_button_press_event (not handled) return false");
	return false; // not handled
}

gboolean gtk_xournal_button_release_event(GtkWidget * widget, GdkEventButton * event) {
#ifdef INPUT_DEBUG
	gboolean isCore = (event->device == gdk_device_get_core_pointer());
	INPUTDBG("ButtonRelease (%s) (x,y)=(%.2f,%.2f), button %d, modifier %x, isCore %i", gdk_device_get_name(event->device), event->x, event->y,
			event->button, event->state, isCore);
#endif
	XInputUtils::fixXInputCoords((GdkEvent*) event, widget);

	if (event->button > 3) { // scroll wheel events
		return true;
	}

	GtkXournal * xournal = GTK_XOURNAL(widget);

	Cursor * cursor = xournal->view->getCursor();
	ToolHandler * h = xournal->view->getControl()->getToolHandler();
	cursor->setMouseDown(false);

	xournal->inScrolling = false;

	EditSelection * sel = xournal->view->getSelection();
	if (sel) {
		sel->mouseUp();
	}

	bool res = false;
	if (xournal->currentInputPage) {
		xournal->currentInputPage->translateEvent((GdkEvent*) event, xournal->x, xournal->y);
		res = xournal->currentInputPage->onButtonReleaseEvent(widget, event);
		xournal->currentInputPage = NULL;
	}

	EditSelection * tmpSelection = xournal->selection;
	xournal->selection = NULL;

	h->restoreLastConfig();

	// we need this workaround so it's possible to select something with the middle button
	if (tmpSelection) {
		xournal->view->setSelection(tmpSelection);
	}

	return res;
}

gboolean gtk_xournal_motion_notify_event(GtkWidget * widget, GdkEventMotion * event) {
#ifdef INPUT_DEBUG
		bool is_core = (event->device == gdk_device_get_core_pointer());
		INPUTDBG("MotionNotify (%s) (x,y)=(%.2f,%.2f), modifier %x", is_core ? "core" : "xinput", event->x, event->y, event->state);
#endif

	XInputUtils::fixXInputCoords((GdkEvent*) event, widget);

	GtkXournal * xournal = GTK_XOURNAL(widget);
	ToolHandler * h = xournal->view->getControl()->getToolHandler();

	if (h->getToolType() == TOOL_HAND) {
		if (xournal->inScrolling) {
			gtk_xournal_scroll_mouse_event(xournal, event);
			return true;
		}
		return false;
	} else if (xournal->selection) {
		EditSelection * selection = xournal->selection;

		PageView * view = selection->getView();
		GdkEventMotion ev = *event;
		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);

		if (xournal->selection->isMoving()) {
			selection->mouseMove(ev.x, ev.y);
		} else {
			CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y, xournal->view->getZoom());
			xournal->view->getCursor()->setMouseSelectionType(selType);
		}
		return true;
	}

	PageView * pv = gtk_xournal_get_page_view_for_pos_cached(xournal, event->x, event->y);
	xournal->view->getCursor()->setInsidePage(pv != NULL);
	if (pv) {
		// allow events only to a single page!
		if (xournal->currentInputPage == NULL || pv == xournal->currentInputPage) {
			pv->translateEvent((GdkEvent*) event, xournal->x, xournal->y);
			return pv->onMotionNotifyEvent(widget, event);;
		}
	}

	return false;
}

static void gtk_xournal_init(GtkXournal * xournal) {
	GtkWidget * widget = GTK_WIDGET(xournal);

	GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_FOCUS);

	int events = GDK_EXPOSURE_MASK;
	events |= GDK_POINTER_MOTION_MASK;
	events |= GDK_EXPOSURE_MASK;
	events |= GDK_BUTTON_MOTION_MASK;
	events |= GDK_BUTTON_PRESS_MASK;
	events |= GDK_BUTTON_RELEASE_MASK;
	events |= GDK_ENTER_NOTIFY_MASK;
	events |= GDK_LEAVE_NOTIFY_MASK;
	events |= GDK_KEY_PRESS_MASK;

	gtk_widget_set_events(widget, events);
}

// gtk_widget_get_preferred_size()
// the output is the default size on window creation
static void gtk_xournal_size_request(GtkWidget * widget, GtkRequisition * requisition) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 200;
	requisition->height = 200;
}

static void gtk_xournal_size_allocate(GtkWidget * widget, GtkAllocation * allocation) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize(widget->window, allocation->x, allocation->y, allocation->width, allocation->height);
	}

	GtkXournal * xournal = GTK_XOURNAL(widget);

	xournal->layout->setSize(allocation->width, allocation->height);
}

static void gtk_xournal_set_adjustment_upper(GtkAdjustment * adj, gdouble upper, gboolean always_emit_changed) {
	gboolean changed = FALSE;
	gboolean value_changed = FALSE;

	gdouble min = MAX(0., upper - adj->page_size);

	if (upper != adj->upper) {
		adj->upper = upper;
		changed = TRUE;
	}

	if (adj->value > min) {
		adj->value = min;
		value_changed = TRUE;
	}

	if (changed || always_emit_changed) {
		gtk_adjustment_changed(adj);
	}
	if (value_changed) {
		gtk_adjustment_value_changed(adj);
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
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);
	gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &widget->style->dark[GTK_STATE_NORMAL]);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

	gtk_xournal_update_xevent(widget);
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

static void gtk_xournal_draw_shadow(GtkXournal * xournal, cairo_t * cr, int left, int top, int width, int height, bool selected) {
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

cairo_t * gtk_xournal_create_cairo_for(GtkWidget * widget, PageView * view) {
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), FALSE);

	GtkXournal * xournal = GTK_XOURNAL(widget);
	double zoom = xournal->view->getZoom();

	// TODO LOW PRIO: stroke draw to this cairo surface look a little different than rendererd to a cairo surface
	cairo_t * cr = gdk_cairo_create(GTK_WIDGET(widget)->window);
	int x = view->getX() - xournal->x;
	int y = view->getY() - xournal->y;
	cairo_translate(cr, x, y);
	cairo_scale(cr, zoom, zoom);

	return cr;
}

void gtk_xournal_repaint_area(GtkWidget * widget, int x1, int y1, int x2, int y2) {
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(widget));

	GtkXournal * xournal = GTK_XOURNAL(widget);

	x1 -= xournal->x;
	x2 -= xournal->x;
	y1 -= xournal->y;
	y2 -= xournal->y;

	if (x2 < 0 || y2 < 0) {
		return; // outside visible area
	}

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(widget, &alloc);

	if (x1 > alloc.width || y1 > alloc.height) {
		return; // outside visible area
	}

	gtk_widget_queue_draw_area(widget, x1, y1, x2 - x1, y2 - y1);
}

static gboolean gtk_xournal_expose(GtkWidget * widget, GdkEventExpose * event) {
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_XOURNAL(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	GtkXournal * xournal = GTK_XOURNAL(widget);

	cairo_t * cr = gdk_cairo_create(GTK_WIDGET(widget)->window);

	ArrayIterator<PageView *> it = xournal->view->pageViewIterator();

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(widget, &alloc);
	int lastVisibleX = alloc.width + xournal->x + 10;
	int lastVisibleY = alloc.height + xournal->y + 10; //+10 fix to draw the shadow

	int firstVisibleX = xournal->x - 10;
	int firstVisibleY = xournal->y - 10;

	while (it.hasNext()) {
		PageView * pv = it.next();

		int px = pv->getX();
		int py = pv->getY();
		int pw = pv->getDisplayWidth();
		int ph = pv->getDisplayHeight();

		// not visible, its on the right side of the visible area
		if (px > lastVisibleX) {
			continue;
		}
		// not visible, its on the left side of the visible area
		if (px + pw < firstVisibleX) {
			continue;
		}
		// not visible, its on the bottom side of the visible area
		if (py > lastVisibleY) {
			continue;
		}
		// not visible, its on the top side of the visible area
		if (py + ph < firstVisibleY) {
			continue;
		}

		int x = px - xournal->x;
		int y = py - xournal->y;

		gtk_xournal_draw_shadow(xournal, cr, x, y, pw, ph, pv->isSelected());
		cairo_save(cr);
		cairo_translate(cr, x, y);

		GdkRectangle rect = event->area;
		rect.x -= x;
		rect.y -= y;

		pv->paintPage(cr, &rect);
		cairo_restore(cr);
	}

	if (xournal->selection) {
		double zoom = xournal->view->getZoom();

		int px = xournal->selection->getXOnView() * zoom;
		int py = xournal->selection->getYOnView() * zoom;
		//		int pw = xournal->selection->getWidth() * zoom;
		//		int ph = xournal->selection->getHeight() * zoom;

		// not visible, its on the right side of the visible area
		if (px > lastVisibleX) {
			printf("test1\n");
		} else
		// not visible, its on the left side of the visible area

		// TODO LOW PRIO this is not working correct if the zoom is small, xournal->x is never smaller than 0
		//		if (px + pw < firstVisibleX) {
		//			printf("test2\n");
		//		} else
		// not visible, its on the bottom side of the visible area
		if (py > lastVisibleY) {
			printf("test3\n");
			//		} else
			//		// not visible, its on the top side of the visible area
			//		if (py + ph < firstVisibleY) {
			//			printf("test4 %i:: %i\n", py + ph, firstVisibleY);
		} else {
			Redrawable * red = xournal->selection->getView();
			cairo_translate(cr, red->getX() - xournal->x, red->getY() - xournal->y);

			xournal->selection->paint(cr, zoom);
		}
	}

	cairo_destroy(cr);

	return true;
}

static void gtk_xournal_destroy(GtkObject * object) {
	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_XOURNAL(object));

	GtkXournal * xournal = GTK_XOURNAL(object);
	delete xournal->pagePositionCache;
	xournal->pagePositionCache = NULL;

	delete xournal->selection;
	xournal->selection = NULL;

	delete xournal->layout;
	xournal->layout = NULL;

	GtkXournalClass * klass = (GtkXournalClass *) gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(*GTK_OBJECT_CLASS(klass)->destroy)(object);
	}
}

