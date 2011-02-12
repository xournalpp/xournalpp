#include "PageView.h"
#include "XournalWidget.h"
#include "../gettext.h"
#include <stdlib.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "../control/Control.h"
#include "../view/TextView.h"
#include "../control/tools/Selection.h"
#include "../util/pixbuf-utils.h"
#include "../util/Range.h"
#include "../cfg.h"
#include "../undo/InsertUndoAction.h"

//#define INPUT_DEBUG

#define EPSILON 1E-7

PageView::PageView(XournalWidget * xournal, XojPage * page) {
	this->page = page;
	this->xournal = xournal;
	this->selected = false;
	this->settings = xournal->getControl()->getSettings();
	this->view = new DocumentView();
	this->lastVisibelTime = -1;

	this->lastMousePositionX = 0;
	this->lastMousePositionY = 0;

	this->repaintX = -1;
	this->repaintY = -1;
	this->repaintWidth = -1;
	this->repaintHeight = -1;

	this->scrollOffsetX = 0;
	this->scrollOffsetY = 0;

	this->inScrolling = false;

	this->firstPainted = false;

	this->crBuffer = NULL;

	this->idleRepaintId = 0;

	this->inEraser = false;

	this->extendedWarningDisplayd = false;

	this->verticalSpace = NULL;

	this->selectionEdit = NULL;
	this->widget = gtk_drawing_area_new();
	gtk_widget_show(this->widget);

	this->textEditor = NULL;

	this->search = NULL;

	this->eraser = new EraseHandler(xournal->getControl()->getUndoRedoHandler(), this->page,
			xournal->getControl()->getToolHandler(), this);

	this->inputHandler = new InputHandler(xournal, this->widget, this);

	updateSize();

	gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK
			| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

	gtk_widget_set_can_focus(widget, true);

	g_signal_connect(widget, "button_press_event", G_CALLBACK(onButtonPressEventCallback), this);
	g_signal_connect(widget, "button_release_event", G_CALLBACK(onButtonReleaseEventCallback), this);
	g_signal_connect(widget, "motion_notify_event", G_CALLBACK(onMotionNotifyEventCallback), this);
	g_signal_connect (widget, "enter_notify_event", G_CALLBACK(onMouseEnterNotifyEvent), NULL);
	g_signal_connect (widget, "leave_notify_event", G_CALLBACK(onMouseLeaveNotifyEvent), NULL);

	g_signal_connect(G_OBJECT(widget), "expose_event", G_CALLBACK(exposeEventCallback), this);

	updateXEvents();
}

PageView::~PageView() {
	gtk_widget_destroy(widget);
	delete this->view;
	delete this->eraser;
	delete this->inputHandler;
	endText();
	deleteViewBuffer();
}

int PageView::getLastVisibelTime() {
	return this->lastVisibelTime;
}

int PageView::getBufferPixels() {
	if (crBuffer) {
		return cairo_image_surface_get_width(crBuffer) * cairo_image_surface_get_height(crBuffer);
	}
	return 0;
}

void PageView::setIsVisibel(bool visibel) {
	if (visibel) {
		this->lastVisibelTime = -1;
	} else if (this->lastVisibelTime == -1) {
		GTimeVal val;
		g_get_current_time(&val);
		this->lastVisibelTime = val.tv_sec;
	}
}

void PageView::deleteViewBuffer() {
	if (this->crBuffer) {
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
}

/**
 * Change event handling between XInput and Core
 */
void PageView::updateXEvents() {
	if (!gtk_check_version(2, 17, 0)) {
		/* GTK+ 2.17 and later: everybody shares a single native window,
		 so we'll never get any core events, and we might as well set
		 extension events the way we're supposed to. Doing so helps solve
		 crasher bugs in 2.17, and prevents us from losing two-button
		 events in 2.18 */
		gtk_widget_set_extension_events(widget, settings->isUseXInput() ? GDK_EXTENSION_EVENTS_ALL
				: GDK_EXTENSION_EVENTS_NONE);
	} else {
		/* GTK+ 2.16 and earlier: we only activate extension events on the
		 PageViews's parent GdkWindow. This allows us to keep receiving core
		 events. */
		gdk_input_set_extension_events(widget->window, GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK
				| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK, settings->isUseXInput() ? GDK_EXTENSION_EVENTS_ALL
				: GDK_EXTENSION_EVENTS_NONE);
	}

}

gboolean PageView::onButtonPressEventCallback(GtkWidget *widget, GdkEventButton *event, PageView * view) {
	view->onButtonPressEvent(widget, event);
	return false;
}

void PageView::handleScrollEvent(GdkEventButton *event) {
	GdkEvent scrollEvent;
	/* with GTK+ 2.17 and later, the entire widget hierarchy is xinput-aware,
	 so the core button event gets discarded and the scroll event never
	 gets processed by the main window. This is arguably a GTK+ bug.
	 We work around it. */
	scrollEvent.scroll.type = GDK_SCROLL;
	scrollEvent.scroll.window = event->window;
	scrollEvent.scroll.send_event = event->send_event;
	scrollEvent.scroll.time = event->time;
	scrollEvent.scroll.x = event->x;
	scrollEvent.scroll.y = event->y;
	scrollEvent.scroll.state = event->state;
	scrollEvent.scroll.device = event->device;
	scrollEvent.scroll.x_root = event->x_root;
	scrollEvent.scroll.y_root = event->y_root;
	if (event->button == 4) {
		scrollEvent.scroll.direction = GDK_SCROLL_UP;
	} else if (event->button == 5) {
		scrollEvent.scroll.direction = GDK_SCROLL_DOWN;
	} else if (event->button == 6) {
		scrollEvent.scroll.direction = GDK_SCROLL_LEFT;
	} else {
		scrollEvent.scroll.direction = GDK_SCROLL_RIGHT;
	}
	gtk_widget_event(gtk_widget_get_parent(xournal->getWidget()), &scrollEvent);

}

bool PageView::searchTextOnPage(const char * text, int * occures, double * top) {
	if (this->search == NULL) {
		if (text == NULL) {
			return true;
		}

		int pNr = page->getPdfPageNr();
		XojPopplerPage * pdf = NULL;
		if (pNr != -1) {
			pdf = xournal->getControl()->getDocument()->getPdfPage(pNr);
		}
		this->search = new SearchControl(page, pdf);
	}

	bool found = this->search->search(text, occures, top);

	gtk_widget_queue_draw(widget);

	return found;
}

void PageView::endText() {
	if (!this->textEditor) {
		return;
	}
	Text * txt = this->textEditor->getText();
	Layer * layer = page->getSelectedLayer();
	UndoRedoHandler * undo = xournal->getControl()->getUndoRedoHandler();

	// Text deleted
	if (txt->getText().isEmpty()) {
		// old element
		int pos = layer->indexOf(txt);
		if (pos != -1) {
			DeleteUndoAction * eraseDeleteUndoAction = new DeleteUndoAction(page, this, true);
			layer->removeElement(txt, false);
			eraseDeleteUndoAction->addElement(layer, txt, pos);
			undo->addUndoAction(eraseDeleteUndoAction);
			this->repaint();
		}
	} else {
		// new element
		if (layer->indexOf(txt) == -1) {
			undo->addUndoActionBefore(new InsertUndoAction(page, layer, txt, this),
					this->textEditor->getFirstUndoAction());
			layer->addElement(txt);
			this->textEditor->textCopyed();
			this->repaint();
		}
	}

	delete this->textEditor;
	this->textEditor = NULL;
	repaint();
}

void PageView::startText(double x, double y) {
	if (this->textEditor == NULL) {
		// Is there already a textfield?
		ListIterator<Element *> eit = page->getSelectedLayer()->elementIterator();

		Text * text = NULL;

		while (eit.hasNext()) {
			Element * e = eit.next();

			if (e->getType() == ELEMENT_TEXT) {
				GdkRectangle matchRect = { x - 10, y - 10, 20, 20 };
				if (e->intersectsArea(&matchRect)) {
					text = (Text *) e;
					break;
				}
			}
		}

		bool ownText = false;
		if (text == NULL) {
			ToolHandler * h = xournal->getControl()->getToolHandler();
			ownText = true;
			text = new Text();
			text->setX(x);
			text->setY(y);
			text->setColor(h->getColor());
			text->setFont(settings->getFont());
		}

		this->textEditor = new TextEditor(this, text, ownText);
		if (!ownText) {
			this->textEditor->mousePressed(x - text->getX(), y - text->getY());
		}

		repaint();
	} else {
		Text * text = this->textEditor->getText();
		GdkRectangle matchRect = { x - 10, y - 10, 20, 20 };
		if (!text->intersectsArea(&matchRect)) {
			endText();
		} else {
			this->textEditor->mousePressed(x - text->getX(), y - text->getY());
		}
	}
}

void PageView::selectObjectAt(double x, double y) {
	int selected = page->getSelectedLayerId();
	GdkRectangle matchRect = { x - 10, y - 10, 20, 20 };

	Stroke * strokeMatch = NULL;
	double gap = 1000000000;

	Element * elementMatch = NULL;

	// clear old selection anyway
	xournal->getControl()->clearSelection();

	ListIterator<Layer*> it = page->layerIterator();
	while (it.hasNext() && selected) {
		Layer * l = it.next();

		ListIterator<Element *> eit = l->elementIterator();
		while (eit.hasNext()) {
			Element * e = eit.next();
			if (e->intersectsArea(&matchRect)) {
				if (e->getType() == ELEMENT_STROKE) {
					Stroke * s = (Stroke *) e;
					double tmpGap = 0;
					if (s->intersects(x, y, 5, &tmpGap)) {
						if (gap > tmpGap) {
							gap = tmpGap;
							strokeMatch = s;
						}
					}
				} else {
					elementMatch = e;
				}
			}
		}

		selected--;
	}

	if (strokeMatch) {
		elementMatch = strokeMatch;
	}

	if (elementMatch) {
		Control * control = xournal->getControl();
		control->setSelection(new EditSelection(control->getUndoRedoHandler(), elementMatch, this, page));
		gtk_widget_queue_draw(this->widget);
	}
}

void PageView::onButtonPressEvent(GtkWidget * widget, GdkEventButton * event) {
#ifdef INPUT_DEBUG
	/**
	 * true: Core event, false: XInput event
	 */
	gboolean isCore = (event->device == gdk_device_get_core_pointer());

	//	printf("DEBUG: ButtonPress (%s) (x,y)=(%.2f,%.2f), button %d, modifier %x, isCore %i\n", event->device->name,
	//			event->xScreen, event->yScreen, event->button, event->state, isCore);
#endif

	fixXInputCoords((GdkEvent*) event);

	xournal->resetFocus();

	if (event->type != GDK_BUTTON_PRESS) {
		return;
	}

	if (!this->selected) {
		xournal->getControl()->firePageSelected(this->page);
	}

	if (event->button > 3) { // scroll wheel events! don't paint...
		handleScrollEvent(event);
		return;
	}
	if ((event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0) {
		return;
	}

	// Change the tool depending on the key or device

	ToolHandler * h = xournal->getControl()->getToolHandler();
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
		if (cfg->disableDrawing && cfg->action == TOOL_NONE) {
			ToolType tool = h->getToolType();
			if (tool == TOOL_PEN || tool == TOOL_ERASER || tool == TOOL_HILIGHTER) {
				printf("ignore touchscreen for drawing!\n");
				return;
			}
		}
	}

	if (cfg && cfg->action != TOOL_NONE) {
		h->copyCurrentConfig();
		h->selectTool(cfg->action);

		ToolType type = cfg->action;

		if (type == TOOL_PEN || type == TOOL_HILIGHTER) {
			h->setRuler(cfg->rouler);
			h->setShapeRecognizer(cfg->shapeRecognizer);
			if (cfg->size != TOOL_SIZE_NONE) {
				h->setSize(cfg->size);
			}
		}

		if (type == TOOL_PEN || type == TOOL_HILIGHTER || type == TOOL_TEXT) {
			h->setColor(cfg->color);
		}

		if (type == TOOL_ERASER && cfg->eraserMode != ERASER_TYPE_NONE) {
			xournal->getControl()->setEraserType(cfg->eraserMode);
		}

	}

	double x;
	double y;

	if (!gdk_event_get_coords((GdkEvent *) event, &x, &y)) {
		return;
	}

	if ((x < 0 || y < 0) && !extendedWarningDisplayd && settings->isXinputEnabled()) {

		GtkWidget
				* dialog =
						gtk_message_dialog_new(
								(GtkWindow *) *xournal->getControl()->getWindow(),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_NONE,
								_("There was a wrong input event, input is not working.\nDo you want to disable \"Extended Input\"?"));

		gtk_dialog_add_button(GTK_DIALOG(dialog), "Disable \"Extended Input\"", 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", 2);

		extendedWarningDisplayd = true;

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == 1) {
			settings->setXinputEnabled(false);
			xournal->updateXEvents();
		}
		gtk_widget_destroy(dialog);
		return;
	}

	double zoom = xournal->getZoom();
	x /= zoom;
	y /= zoom;

	if (h->getToolType() == TOOL_PEN) {
		this->inputHandler->startStroke(event, STROKE_TOOL_PEN, x, y);
	} else if (h->getToolType() == TOOL_HAND) {
		this->lastMousePositionX = 0;
		this->lastMousePositionY = 0;
		this->inScrolling = true;
		gtk_widget_get_pointer(widget, &this->lastMousePositionX, &this->lastMousePositionY);
	} else if (h->getToolType() == TOOL_HILIGHTER) {
		this->inputHandler->startStroke(event, STROKE_TOOL_HIGHLIGHTER, x, y);
	} else if (h->getToolType() == TOOL_ERASER) {
		if (h->getEraserType() == ERASER_TYPE_WHITEOUT) {
			this->inputHandler->startStroke(event, STROKE_TOOL_ERASER, x, y);
			this->inputHandler->getTmpStroke()->setColor(0xffffff); // White
		} else {
			this->eraser->erase(x, y);
			this->inEraser = true;
		}
	} else if (h->getToolType() == TOOL_VERTICAL_SPACE) {
		this->verticalSpace = new VerticalToolHandler(this, this->page, y, zoom);
	} else if (h->getToolType() == TOOL_SELECT_RECT || h->getToolType() == TOOL_SELECT_REGION || h->getToolType()
			== TOOL_SELECT_OBJECT) {
		if (xournal->getControl()->getSelectionFor(this)) {
			EditSelection * selection = xournal->getControl()->getSelectionFor(this);
			CursorSelectionType selType = selection->getSelectionTypeForPos(event->x, event->y, zoom);
			if (selType) {
				selection->setEditMode(selType, event->x / zoom, event->y / zoom);
				return;
			} else {
				xournal->getControl()->clearSelection();
			}
		}
		if (h->getToolType() == TOOL_SELECT_RECT) {
			if (this->selectionEdit) {
				delete this->selectionEdit;
				this->selectionEdit = NULL;
			}
			this->selectionEdit = new RectSelection(x, y, this);
		} else if (h->getToolType() == TOOL_SELECT_REGION) {
			if (this->selectionEdit) {
				delete this->selectionEdit;
				this->selectionEdit = NULL;
			}
			this->selectionEdit = new RegionSelect(x, y, this);
		} else if (h->getToolType() == TOOL_SELECT_OBJECT) {
			selectObjectAt(x, y);
		}
	} else if (h->getToolType() == TOOL_TEXT) {
		startText(x, y);
	} else if (h->getToolType() == TOOL_IMAGE) {
		insertImage(x, y);
	}

	Cursor * cursor = xournal->getControl()->getCursor();
	cursor->setMouseDown(true);
}

class InsertImageRunnable: public Runnable {
public:
	InsertImageRunnable(PageView * view, GFile * file, double x, double y) {
		this->view = view;
		this->x = x;
		this->y = y;
		this->file = file;
	}
	~InsertImageRunnable() {
		g_object_unref(file);
	}

	bool run(bool * cancel) {
		GError * err = NULL;
		GFileInputStream * in = g_file_read(file, NULL, &err);
		GdkPixbuf * pixbuf = NULL;

		if (!err) {
			pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), NULL, &err);
			g_input_stream_close(G_INPUT_STREAM(in), NULL, NULL);
		}

		if (err) {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *view->xournal->getControl()->getWindow(),
					GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("This image could not be loaded. Error message: %s"), err->message);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			g_error_free(err);
			return false;
		}

		Image * img = new Image();
		img->setX(x);
		img->setY(y);
		img->setImage(f_pixbuf_to_cairo_surface(pixbuf));

		int width = gdk_pixbuf_get_width(pixbuf);
		int height = gdk_pixbuf_get_height(pixbuf);
		gdk_pixbuf_unref(pixbuf);

		double zoom = 1;

		if (x + width > view->page->getWidth() || y + height > view->page->getHeight()) {
			double maxZoomX = (view->page->getWidth() - x) / width;
			double maxZoomY = (view->page->getHeight() - y) / height;

			if (maxZoomX < maxZoomY) {
				zoom = maxZoomX;
			} else {
				zoom = maxZoomY;
			}
		}

		img->setWidth(width * zoom);
		img->setHeight(height * zoom);

		view->page->getSelectedLayer()->addElement(img);
		view->repaint();
	}

private:
	PageView * view;
	double x;
	double y;
	GFile * file;
};

void PageView::insertImage(double x, double y) {
	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open Image"), (GtkWindow*) *xournal->getControl()->getWindow(),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

	// here we can handle remote files without problems with backward compatibility
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);

	GtkFileFilter *filterSupported = gtk_file_filter_new();
	gtk_file_filter_set_name(filterSupported, _("Images"));
	gtk_file_filter_add_pixbuf_formats(filterSupported);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

	if (!settings->getLastSavePath().isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}

	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return;
	}
	GFile * file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));

	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastSavePath(folder);
	g_free(folder);

	gtk_widget_destroy(dialog);

	xournal->getControl()->runInBackground(new InsertImageRunnable(this, file, x, y));
}

void PageView::redrawDocumentRegion(double x1, double y1, double x2, double y2) {
	double zoom = xournal->getZoom();
	gtk_widget_queue_draw_area(this->widget, x1 * zoom - 10, y1 * zoom - 10, (x2 - x1) * zoom + 20, (y2 - y1) * zoom
			+ 20);
}

GdkColor PageView::getSelectionColor() {
	return widget->style->base[GTK_STATE_SELECTED];
}

TextEditor * PageView::getTextEditor() {
	return textEditor;
}

gboolean PageView::onMouseEnterNotifyEvent(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	GList *dev_list;
	GdkDevice *dev;

#ifdef INPUT_DEBUG
	printf("DEBUG: enter notify\n");
#endif
	/* re-enable input devices after they've been emergency-disabled
	 by leave_notify */
	if (!gtk_check_version(2, 17, 0)) {
		gdk_flush();
		gdk_error_trap_push();
		for (dev_list = gdk_devices_list(); dev_list != NULL; dev_list = dev_list->next) {
			dev = GDK_DEVICE(dev_list->data);
			gdk_device_set_mode(dev, GDK_MODE_SCREEN);
		}
		gdk_flush();
		gdk_error_trap_pop();
	}
	return FALSE;
}

gboolean PageView::onMouseLeaveNotifyEvent(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
	GList *dev_list;
	GdkDevice *dev;

#ifdef INPUT_DEBUG
	printf("DEBUG: leave notify (mode=%d, details=%d)\n", event->mode, event->detail);
#endif
	/* emergency disable XInput to avoid segfaults (GTK+ 2.17) or
	 interface non-responsiveness (GTK+ 2.18) */
	if (!gtk_check_version(2, 17, 0)) {
		gdk_flush();
		gdk_error_trap_push();
		for (dev_list = gdk_devices_list(); dev_list != NULL; dev_list = dev_list->next) {
			dev = GDK_DEVICE(dev_list->data);
			gdk_device_set_mode(dev, GDK_MODE_DISABLED);
		}
		gdk_flush();
		gdk_error_trap_pop();
	}
	return FALSE;
}

gboolean PageView::onMotionNotifyEventCallback(GtkWidget *widget, GdkEventMotion *event, PageView * view) {
	CHECK_MEMORY(view);
	return view->onMotionNotifyEvent(widget, event);
}

gboolean PageView::onMotionNotifyEvent(GtkWidget * widget, GdkEventMotion * event) {
#ifdef INPUT_DEBUG
	bool is_core = (event->device == gdk_device_get_core_pointer());
	//	printf("DEBUG: MotionNotify (%s) (x,y)=(%.2f,%.2f), modifier %x\n", is_core ? "core" : "xinput", event->xScreen,
	//			event->yScreen, event->state);
#endif

	fixXInputCoords((GdkEvent*) event);

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;
	ToolHandler * h = xournal->getControl()->getToolHandler();
	if (h->getToolType() == TOOL_HAND) {
		if (this->inScrolling) {
			doScroll(event);
		}
	} else if (h->getToolType() == TOOL_ERASER && h->getEraserType() != ERASER_TYPE_WHITEOUT && this->inEraser) {
		this->eraser->erase(x, y);
	} else {
		if (this->inputHandler->onMotionNotifyEvent(event)) {
		} else if (this->selectionEdit) {
			this->selectionEdit->currentPos(x, y);
		} else if (this->verticalSpace) {
			this->verticalSpace->currentPos(x, y);
		} else if (xournal->getControl()->getSelectionFor(this)) {
			EditSelection * selection = xournal->getControl()->getSelectionFor(this);
			if (selection->getEditMode()) {
				selection->move(x, y, this, xournal);
			} else {
				Cursor * cursor = xournal->getControl()->getCursor();

				CursorSelectionType selType = selection->getSelectionTypeForPos(event->x, event->y, zoom);
				cursor->setMouseSelectionType(selType);
			}
		} else if (this->textEditor) {
			Cursor * cursor = getXournal()->getControl()->getCursor();
			cursor->setInvisible(false);

			Text * text = this->textEditor->getText();
			this->textEditor->mouseMoved(x - text->getX(), y - text->getY());
		}
	}

	return false;
}

bool PageView::scrollCallback(PageView * view) {
	view->xournal->getControl()->scrollRelative(view->scrollOffsetX, view->scrollOffsetY);

	view->scrollOffsetX = 0;
	view->scrollOffsetY = 0;

	return false;
}

void PageView::doScroll(GdkEventMotion * event) {
	int x = 0;
	int y = 0;
	gtk_widget_get_pointer(widget, &x, &y);

	if (this->lastMousePositionX - x == 0 && this->lastMousePositionY - y == 0) {
		return;
	}

	if (this->scrollOffsetX == 0 && this->scrollOffsetY == 0) {
		g_idle_add((GSourceFunc) scrollCallback, this);
	}

	this->scrollOffsetX = this->lastMousePositionX - x;
	this->scrollOffsetY = this->lastMousePositionY - y;
}

bool PageView::onButtonReleaseEventCallback(GtkWidget * widget, GdkEventButton * event, PageView * view) {
	CHECK_MEMORY(view);
	return view->onButtonReleaseEvent(widget, event);
}

bool PageView::onButtonReleaseEvent(GtkWidget * widget, GdkEventButton * event) {
	gboolean isCore = (event->device == gdk_device_get_core_pointer());
#ifdef INPUT_DEBUG
	//	printf("DEBUG: ButtonRelease (%s) (x,y)=(%.2f,%.2f), button %d, modifier %x, isCore %i\n", event->device->name,
	//			event->xScreen, event->yScreen, event->button, event->state, isCore);
#endif

	fixXInputCoords((GdkEvent*) event);
	Control * control = xournal->getControl();

	this->inputHandler->onButtonReleaseEvent(event, this->page);

	ToolHandler * h = control->getToolHandler();
	h->restoreLastConfig();

	Cursor * cursor = control->getCursor();
	cursor->setMouseDown(false);

	this->inScrolling = false;

	if (this->inEraser) {
		this->inEraser = false;
		this->eraser->finalize();
	}

	if (this->verticalSpace) {
		MoveUndoAction * undo = this->verticalSpace->finalize();
		delete this->verticalSpace;
		this->verticalSpace = NULL;
		control->getUndoRedoHandler()->addUndoAction(undo);
	}

	EditSelection * sel = control->getSelectionFor(this);

	if (this->selectionEdit) {
		if (this->selectionEdit->finalize(this->page)) {
			control->setSelection(new EditSelection(control->getUndoRedoHandler(), this->selectionEdit, this));
		}
		delete this->selectionEdit;
		this->selectionEdit = NULL;
	} else if (sel) {
		CHECK_MEMORY(sel);

		sel->finalizeEditing();
	} else if (this->textEditor) {
		this->textEditor->mouseReleased();
	}

	return false;
}

bool PageView::onKeyPressEvent(GdkEventKey *event) {
	// Esc leaves text edition
	if (event->keyval == GDK_Escape) {
		if (this->textEditor) {
			endText();
			return true;
		} else if (xournal->getControl()->getSelection()) {
			xournal->getControl()->clearSelection();
			return true;
		} else {
			return false;
		}
	}

	if (this->textEditor && this->textEditor->onKeyPressEvent(event)) {
		return true;
	}

	EditSelection * selection = xournal->getControl()->getSelectionFor(this);
	if (selection) {
		int d = 10;

		if (event->state & GDK_MOD1_MASK || event->state & GDK_SHIFT_MASK) {
			d = 1;
		}

		if (event->keyval == GDK_Left) {
			selection->doMove(-d, 0, this, xournal);
			return true;
		} else if (event->keyval == GDK_Up) {
			selection->doMove(0, -d, this, xournal);
			return true;
		} else if (event->keyval == GDK_Right) {
			selection->doMove(d, 0, this, xournal);
			return true;
		} else if (event->keyval == GDK_Down) {
			selection->doMove(0, d, this, xournal);
			return true;
		}
	}

	return false;
}

bool PageView::onKeyReleaseEvent(GdkEventKey *event) {
	if (this->textEditor && this->textEditor->onKeyReleaseEvent(event)) {
		return true;
	}

	return false;
}

GtkWidget * PageView::getWidget() {
	return widget;
}

XournalWidget * PageView::getXournal() {
	return xournal;
}

double PageView::getHeight() {
	return page->getHeight();
}

double PageView::getWidth() {
	return page->getWidth();
}

int PageView::getDisplayWidth() {
	return page->getWidth() * xournal->getZoom();
}

int PageView::getDisplayHeight() {
	return page->getHeight() * xournal->getZoom();
}

gboolean PageView::exposeEventCallback(GtkWidget *widget, GdkEventExpose *event, PageView * page) {
	return page->paintPage(widget, event, page->getXournal()->getZoom());
}

XojPage * PageView::getPage() {
	return page;
}

void PageView::repaint() {
	deleteViewBuffer();
	gtk_widget_queue_draw(widget);
}

void PageView::redraw() {
	gtk_widget_queue_draw(widget);
}

void PageView::repaint(Range & r) {
	repaint(r.getX(), r.getY(), r.getWidth(), r.getHeight());
}

void PageView::repaint(Element * e) {
	repaint(e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
}

void PageView::repaint(double x, double y, double width, double heigth) {
	double zoom = xournal->getZoom();

	this->repaintX = MAX(x - 10,0);
	this->repaintY = MAX(y - 10,0);
	this->repaintWidth = width + 20;
	this->repaintHeight = heigth + 20;

	gtk_widget_queue_draw_area(widget, this->repaintX * zoom, this->repaintY * zoom, this->repaintWidth * zoom,
			this->repaintHeight * zoom);
}

void PageView::updateSize() {
	gtk_widget_set_size_request(widget, getDisplayWidth(), getDisplayHeight());
}

void PageView::setSelected(bool selected) {
	this->selected = selected;

	GtkWidget * parent = gtk_widget_get_parent(this->widget);

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(parent, &alloc);
	gtk_widget_queue_draw(parent);

	if (selected) {
		gtk_widget_grab_focus(widget);
	}
}

void PageView::firstPaint() {
	if (!GDK_IS_WINDOW(this->widget->window)) {
		return;
	}

	this->firstPainted = true;

	gdk_window_set_background(this->widget->window, &this->widget->style->white);
	gtk_widget_queue_draw(this->widget);
}

bool PageView::repaintCallback(PageView * view) {
	view->idleRepaintId = 0;

	view->deleteViewBuffer();
	view->paintPage(view->widget, NULL, view->getXournal()->getZoom());
	return false;
}

void PageView::repaintLater() {
	if (this->idleRepaintId) {
		return;
	}

	this->idleRepaintId = g_idle_add((GSourceFunc) &repaintCallback, this);
}

bool PageView::isSelected() {
	return selected;
}

bool PageView::cut() {
	if (this->textEditor) {
		this->textEditor->cutToClipboard();
		return true;
	}
	return false;
}

bool PageView::copy() {
	if (this->textEditor) {
		this->textEditor->copyToCliboard();
		return true;
	}
	return false;
}

bool PageView::paste() {
	if (this->textEditor) {
		this->textEditor->pasteFromClipboard();
		return true;
	}
	return false;
}

bool PageView::actionDelete() {
	if (this->textEditor) {
		this->textEditor->deleteFromCursor(GTK_DELETE_CHARS, 1);
		return true;
	}
	return false;
}

bool PageView::paintPage(GtkWidget * widget, GdkEventExpose * event, double zoom) {
	if (!firstPainted) {
		firstPaint();
		return true;
	}

	cairo_t * cr = gdk_cairo_create(widget->window);

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	if (this->crBuffer == NULL) {
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
		cairo_t * cr2 = cairo_create(this->crBuffer);

		cairo_scale(cr2, xournal->getZoom(), xournal->getZoom());

		XojPopplerPage * popplerPage = NULL;

		if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
			int pgNo = page->getPdfPageNr();
			popplerPage = xournal->getDocument()->getPdfPage(pgNo);
		}

		view->drawPage(page, popplerPage, cr2);

		cairo_destroy(cr2);

		this->repaintX = -1;
		this->repaintY = -1;
		this->repaintWidth = -1;
		this->repaintHeight = -1;
	}

	if (this->repaintX != -1) {
		cairo_t * crPageBuffer = cairo_create(this->crBuffer);

		XojPopplerPage * popplerPage = NULL;

		if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
			int pgNo = page->getPdfPageNr();
			popplerPage = xournal->getDocument()->getPdfPage(pgNo);
		}

		cairo_surface_t * rectBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->repaintWidth * zoom,
				this->repaintHeight * zoom);
		cairo_t * crRect = cairo_create(rectBuffer);
		cairo_scale(crRect, zoom, zoom);
		cairo_translate(crRect, -this->repaintX, -this->repaintY);

		view->limitArea(this->repaintX, this->repaintY, this->repaintWidth, this->repaintHeight);
		view->drawPage(page, popplerPage, crRect);

		cairo_destroy(crRect);

		cairo_set_operator(crPageBuffer, CAIRO_OPERATOR_SOURCE);
		cairo_set_source_surface(crPageBuffer, rectBuffer, this->repaintX * zoom, this->repaintY * zoom);
		cairo_rectangle(crPageBuffer, this->repaintX * zoom, this->repaintY * zoom, this->repaintWidth * zoom,
				this->repaintHeight * zoom);
		cairo_fill(crPageBuffer);

		cairo_destroy(crPageBuffer);

		cairo_surface_destroy(rectBuffer);

		this->repaintX = -1;
		this->repaintY = -1;
		this->repaintWidth = -1;
		this->repaintHeight = -1;
	}

	cairo_save(cr);

	double width = cairo_image_surface_get_width(this->crBuffer);
	if (width != alloc.width) {
		double scale = ((double) alloc.width) / ((double) width);

		// Scale current image to fit the zoom level
		cairo_scale(cr, scale, scale);
		cairo_set_source_surface(cr, this->crBuffer, 0, 0);

		repaintLater();

		event = NULL;
	} else {
		cairo_set_source_surface(cr, this->crBuffer, 0, 0);
	}

	if (event) {
		cairo_rectangle(cr, event->area.x, event->area.y, event->area.width, event->area.height);
		cairo_fill(cr);
	} else {
		cairo_paint(cr);
	}

	cairo_restore(cr);

	cairo_scale(cr, xournal->getZoom(), xournal->getZoom());

	if (this->textEditor) {
		this->textEditor->paint(cr, event, xournal->getZoom());
	}
	if (this->selectionEdit) {
		this->selectionEdit->paint(cr, event, xournal->getZoom());
	}
	if (this->verticalSpace) {
		this->verticalSpace->paint(cr, event, xournal->getZoom());
	}

	Control * control = xournal->getControl();

	control->paintSelection(cr, event, xournal->getZoom(), this);

	if (this->search) {
		this->search->paint(cr, event, xournal->getZoom(), getSelectionColor());
	}
	this->inputHandler->draw(cr);

	cairo_destroy(cr);
	return true;
}

void PageView::fixXInputCoords(GdkEvent * event) {
	double *axes, *px, *py;
	GdkDevice *device;
	int wx, wy, ix, iy;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE) {
		axes = event->button.axes;
		px = &(event->button.x);
		py = &(event->button.y);
		device = event->button.device;
	} else if (event->type == GDK_MOTION_NOTIFY) {
		axes = event->motion.axes;
		px = &(event->motion.x);
		py = &(event->motion.y);
		device = event->motion.device;
	} else {
		return; // nothing we know how to do
	}

#ifdef ENABLE_XINPUT_BUGFIX
	if (axes == NULL) {
		return;
	}

	// fix broken events with the core pointer's location
	if (!finite(axes[0]) || !finite(axes[1]) || (axes[0] == 0. && axes[1] == 0.)) {
		gdk_window_get_pointer(widget->window, &ix, &iy, NULL);
		*px = ix;
		*py = iy;
	} else {
		GdkScreen * screen = gtk_widget_get_screen(xournal->getWidget());
		int screenWidth = gdk_screen_get_width(screen);
		int screenHeight = gdk_screen_get_height(screen);

		gdk_window_get_origin(widget->window, &wx, &wy);
		double axisWidth = device->axes[0].max - device->axes[0].min;

		if (axisWidth > EPSILON) {
			*px = (axes[0] / axisWidth) * screenWidth - wx;
		}
		axisWidth = device->axes[1].max - device->axes[1].min;
		if (axisWidth > EPSILON) {
			*py = (axes[1] / axisWidth) * screenHeight - wy;
		}

	}
#else
	if (!finite(*px) || !finite(*py) || (*px == 0. && *py == 0.)) {
		gdk_window_get_pointer(widget->window, &ix, &iy, NULL);
		*px = ix;
		*py = iy;
	}
#endif
}

