#include "PageView.h"
#include "XournalWidget.h"
#include <stdlib.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "../control/Control.h"
#include "../view/TextView.h"
#include "../control/tools/Selection.h"
#include "../control/tools/ImageHandler.h"
#include "../util/pixbuf-utils.h"
#include "../util/Range.h"
#include "../util/XInputUtils.h"
#include "../cfg.h"
#include "../undo/InsertUndoAction.h"
#include "../control/jobs/BlockingJob.h"
#include "../model/Image.h"

#include "../model/Page.h"
#include "../model/Stroke.h"
#include "../model/Text.h"

#include "../control/settings/Settings.h"
#include "../control/SearchControl.h"
#include "../control/tools/VerticalToolHandler.h"
#include "../control/tools/EraseHandler.h"
#include "../control/tools/InputHandler.h"
#include "../gui/TextEditor.h"
#include "../util/Rectangle.h"
#include "../undo/DeleteUndoAction.h"

#include <config.h>
#include <glib/gi18n-lib.h>

PageView::PageView(XournalWidget * xournal, XojPage * page) {
	this->page = page;
	this->xournal = xournal;
	this->selected = false;
	this->settings = xournal->getControl()->getSettings();
	this->lastVisibelTime = -1;

	this->drawingMutex = g_mutex_new();

	this->lastMousePositionX = 0;
	this->lastMousePositionY = 0;

	this->repaintRect = NULL;
	this->repaintComplete = false;
	this->repaintRectMutex = g_mutex_new();

	this->scrollOffsetX = 0;
	this->scrollOffsetY = 0;

	this->inScrolling = false;

	this->firstPainted = false;

	this->crBuffer = NULL;

	this->inEraser = false;

	this->extendedWarningDisplayd = false;

	this->verticalSpace = NULL;

	this->selectionEdit = NULL;
	this->widget = gtk_drawing_area_new();

	gtk_widget_show(this->widget);

	this->textEditor = NULL;

	this->search = NULL;

	this->eraser = new EraseHandler(xournal->getControl()->getUndoRedoHandler(), xournal->getControl()->getDocument(), this->page,
			xournal->getControl()->getToolHandler(), this);

	this->inputHandler = new InputHandler(xournal, this->widget, this);

	updateSize();

	gtk_widget_set_events(
			widget,
			GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK
					| GDK_LEAVE_NOTIFY_MASK);

	gtk_widget_set_can_focus(widget, true);

	g_signal_connect(widget, "button_press_event", G_CALLBACK(onButtonPressEventCallback), this);
	g_signal_connect(widget, "button_release_event", G_CALLBACK(onButtonReleaseEventCallback), this);
	g_signal_connect(widget, "motion_notify_event", G_CALLBACK(onMotionNotifyEventCallback), this);
	g_signal_connect(widget, "enter_notify_event", G_CALLBACK(XInputUtils::onMouseEnterNotifyEvent), NULL);
	g_signal_connect(widget, "leave_notify_event", G_CALLBACK(XInputUtils::onMouseLeaveNotifyEvent), NULL);

	g_signal_connect(G_OBJECT(widget), "expose_event", G_CALLBACK(exposeEventCallback), this);

	updateXEvents();
}

PageView::~PageView() {
	this->xournal->getControl()->getScheduler()->removePage(this);

	gtk_widget_destroy(widget);
	delete this->eraser;
	this->eraser = NULL;
	delete this->inputHandler;
	this->inputHandler = NULL;
	endText();
	deleteViewBuffer();

	g_mutex_free(this->repaintRectMutex);
	for (GList * l = this->repaintRect; l != NULL; l = l->next) {
		Rectangle * rect = (Rectangle *) l->data;
		delete rect;
	}
	g_list_free(this->repaintRect);
	this->repaintRect = NULL;

	g_mutex_free(this->drawingMutex);
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
	g_mutex_lock(this->drawingMutex);
	if (this->crBuffer) {
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
	g_mutex_unlock(this->drawingMutex);
}

bool PageView::repaintCallback(PageView * view) {
	gdk_threads_enter();

	view->paintPage(NULL);

	gdk_threads_leave();

	return false; // do not call again
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
		gtk_widget_set_extension_events(widget, settings->isUseXInput() ? GDK_EXTENSION_EVENTS_ALL : GDK_EXTENSION_EVENTS_NONE);
	} else {
		/* GTK+ 2.16 and earlier: we only activate extension events on the
		 PageViews's parent GdkWindow. This allows us to keep receiving core
		 events. */
		gdk_input_set_extension_events(widget->window, GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
				settings->isUseXInput() ? GDK_EXTENSION_EVENTS_ALL : GDK_EXTENSION_EVENTS_NONE);
	}

}

gboolean PageView::onButtonPressEventCallback(GtkWidget * widget, GdkEventButton * event, PageView * view) {
	view->onButtonPressEvent(widget, event);
	return false;
}

bool PageView::searchTextOnPage(const char * text, int * occures, double * top) {
	if (this->search == NULL) {
		if (text == NULL) {
			return true;
		}

		int pNr = page->getPdfPageNr();
		XojPopplerPage * pdf = NULL;
		if (pNr != -1) {
			Document * doc = xournal->getControl()->getDocument();

			doc->lock();
			pdf = doc->getPdfPage(pNr);
			doc->unlock();
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
		}
	} else {
		// new element
		if (layer->indexOf(txt) == -1) {
			undo->addUndoActionBefore(new InsertUndoAction(page, layer, txt, this), this->textEditor->getFirstUndoAction());
			layer->addElement(txt);
			this->textEditor->textCopyed();
		}
	}

	delete this->textEditor;
	this->textEditor = NULL;
	this->repaint();
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

	XInputUtils::fixXInputCoords((GdkEvent*) event, this->widget);

	xournal->resetFocus();

	if (event->type != GDK_BUTTON_PRESS) {
		return;
	}

	if (event->button > 3) { // scroll wheel events! don't paint...
		XInputUtils::handleScrollEvent(event, gtk_widget_get_parent(xournal->getWidget()));
		return;
	}
	if ((event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0) {
		return;
	}

	if (!this->selected) {
		xournal->getControl()->firePageSelected(this->page);
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

		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *xournal->getControl()->getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
				GTK_BUTTONS_NONE, _("There was a wrong input event, input is not working.\nDo you want to disable \"Extended Input\"?"));

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

	Cursor * cursor = xournal->getControl()->getCursor();
	cursor->setMouseDown(true);

	// hand tool don't change the selection, so you can scroll e.g.
	// with your touchscreen without remove the selection
	if (h->getToolType() == TOOL_HAND) {
		this->lastMousePositionX = 0;
		this->lastMousePositionY = 0;
		this->inScrolling = true;
		gtk_widget_get_pointer(widget, &this->lastMousePositionX, &this->lastMousePositionY);
	} else if (xournal->getControl()->getSelectionFor(this)) {
		EditSelection * selection = xournal->getControl()->getSelectionFor(this);
		CursorSelectionType selType = selection->getSelectionTypeForPos(event->x, event->y, zoom);
		if (selType) {
			selection->setEditMode(selType, event->x / zoom, event->y / zoom);
			// don't do everything else
			return;
		} else {
			xournal->getControl()->clearSelection();
		}
	}

	if (h->getToolType() == TOOL_PEN) {
		this->inputHandler->startStroke(event, STROKE_TOOL_PEN, x, y);
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
	} else if (h->getToolType() == TOOL_SELECT_RECT || h->getToolType() == TOOL_SELECT_REGION || h->getToolType() == TOOL_SELECT_OBJECT) {
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
		ImageHandler imgHandler(xournal->getControl(), this);
		imgHandler.insertImage(x, y);
	}
}

void PageView::redraw(double x1, double y1, double x2, double y2) {
	double zoom = xournal->getZoom();
	gtk_widget_queue_draw_area(this->widget, x1 * zoom - 10, y1 * zoom - 10, (x2 - x1) * zoom + 20, (y2 - y1) * zoom + 20);
}

GdkColor PageView::getSelectionColor() {
	return widget->style->base[GTK_STATE_SELECTED];
}

TextEditor * PageView::getTextEditor() {
	return textEditor;
}

void PageView::resetShapeRecognizer() {
	this->inputHandler->resetShapeRecognizer();
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

	XInputUtils::fixXInputCoords((GdkEvent*) event, this->widget);

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;

	ToolHandler * h = xournal->getControl()->getToolHandler();

	if (h->getToolType() == TOOL_HAND) {
		if (this->inScrolling) {
			doScroll(event);
		}
	} else if (this->inputHandler->onMotionNotifyEvent(event)) {
		// input handler used this event
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
	} else if (h->getToolType() == TOOL_ERASER && h->getEraserType() != ERASER_TYPE_WHITEOUT && this->inEraser) {
		this->eraser->erase(x, y);
	}

	return false;
}

bool PageView::scrollCallback(PageView * view) {
	gdk_threads_enter();

	view->xournal->getControl()->getScrollHandler()->scrollRelative(view->scrollOffsetX, view->scrollOffsetY);

	view->scrollOffsetX = 0;
	view->scrollOffsetY = 0;

	gdk_threads_leave();

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

	XInputUtils::fixXInputCoords((GdkEvent*) event, this->widget);
	Control * control = xournal->getControl();

	this->inputHandler->onButtonReleaseEvent(event, this->page);

	ToolHandler * h = control->getToolHandler();
	h->restoreLastConfig();

	Cursor * cursor = control->getCursor();
	cursor->setMouseDown(false);

	this->inScrolling = false;

	if (this->inEraser) {
		this->inEraser = false;
		Document * doc = this->xournal->getControl()->getDocument();
		doc->lock();
		this->eraser->finalize();
		doc->unlock();
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

bool PageView::exposeEventCallback(GtkWidget * widget, GdkEventExpose * event, PageView * page) {
	return page->paintPage(event);
}

XojPage * PageView::getPage() {
	return page;
}

void PageView::repaint() {
	this->repaintComplete = true;
	this->xournal->getControl()->getScheduler()->addRepaintPage(this);
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
	int rx = (int) MAX(x - 10, 0);
	int ry = (int) MAX(y - 10, 0);
	int rwidth = (int) (width + 20);
	int rheight = (int) (heigth + 20);

	addRepaintRect(rx, ry, rwidth, rheight);
}

void PageView::addRepaintRect(double x, double y, double width, double height) {
	if (this->repaintComplete) {
		return;
	}

	Rectangle * rect = new Rectangle(x, y, width, height);

	Rectangle dest;

	g_mutex_lock(this->repaintRectMutex);

	for (GList * l = this->repaintRect; l != NULL; l = l->next) {
		Rectangle * r = (Rectangle *) l->data;

		// its better to redraw only one rect than repaint twice the same area
		if (r->intersect(rect, &dest)) {
			r->x = dest.x;
			r->y = dest.y;
			r->width = dest.width;
			r->height = dest.height;

			delete rect;

			g_mutex_unlock(this->repaintRectMutex);
			return;
		}
	}

	this->repaintRect = g_list_append(this->repaintRect, rect);
	g_mutex_unlock(this->repaintRectMutex);

	this->xournal->getControl()->getScheduler()->addRepaintPage(this);
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

bool PageView::paintPage(GdkEventExpose * event) {
	if (!firstPainted) {
		firstPaint();
		return true;
	}
	double zoom = xournal->getZoom();

	cairo_t * cr = gdk_cairo_create(widget->window);

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	g_mutex_lock(this->drawingMutex);

	if (this->crBuffer == NULL) {
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
		cairo_t * cr2 = cairo_create(this->crBuffer);
		cairo_scale(cr2, xournal->getZoom(), xournal->getZoom());

		const char * txtLoading = _("Loading...");

		cairo_text_extents_t ex;
		cairo_set_source_rgb(cr2, 0.5, 0.5, 0.5);
		cairo_select_font_face(cr2, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr2, 32.0);
		cairo_text_extents(cr2, txtLoading, &ex);
		cairo_move_to(cr2, (page->getWidth() - ex.width) / 2 - ex.x_bearing, (page->getHeight() - ex.height) / 2 - ex.y_bearing);
		cairo_show_text(cr2, txtLoading);

		cairo_destroy(cr2);
		repaint();
	}

	cairo_save(cr);

	double width = cairo_image_surface_get_width(this->crBuffer);
	if (width != alloc.width) {
		double scale = ((double) alloc.width) / ((double) width);

		// Scale current image to fit the zoom level
		cairo_scale(cr, scale, scale);
		cairo_set_source_surface(cr, this->crBuffer, 0, 0);

		repaint();

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

	cairo_scale(cr, zoom, zoom);

	if (this->textEditor) {
		this->textEditor->paint(cr, event, zoom);
	}
	if (this->selectionEdit) {
		this->selectionEdit->paint(cr, event, zoom);
	}
	if (this->verticalSpace) {
		this->verticalSpace->paint(cr, event, zoom);
	}

	Control * control = xournal->getControl();

	control->paintSelection(cr, event, zoom, this);

	if (this->search) {
		this->search->paint(cr, event, zoom, getSelectionColor());
	}
	this->inputHandler->draw(cr, zoom);

	cairo_destroy(cr);

	g_mutex_unlock(this->drawingMutex);
	return true;
}
