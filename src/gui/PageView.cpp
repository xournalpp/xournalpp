#include "PageView.h"

#include "Cursor.h"
#include "RepaintHandler.h"
#include "TextEditor.h"
#include "XournalView.h"

#include "control/Control.h"
#include "control/SearchControl.h"
#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "control/jobs/BlockingJob.h"
#include "control/tools/EraseHandler.h"
#include "control/tools/ImageHandler.h"
#include "control/tools/InputHandler.h"
#include "control/tools/Selection.h"
#include "control/tools/VerticalToolHandler.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "model/PageRef.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "undo/DeleteUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/TextBoxUndoAction.h"
//#include "undo/TextUndoAction.h"	//for the save file undo
#include "view/TextView.h"
#include "widgets/XournalWidget.h"

#include <config.h>
#include <config-debug.h>
#include <i18n.h>
#include <pixbuf-utils.h>
#include <Range.h>
#include <Rectangle.h>

#include <glib.h>
#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <math.h>

PageView::PageView(XournalView* xournal, PageRef page)
{
	XOJ_INIT_TYPE(PageView);

	this->page = page;
	this->registerListener(this->page);
	this->xournal = xournal;
	this->selected = false;
	this->settings = xournal->getControl()->getSettings();
	this->lastVisibleTime = -1;

	g_mutex_init(&this->drawingMutex);

	this->rerenderComplete = false;
	g_mutex_init(&this->repaintRectMutex);

	this->crBuffer = NULL;

	this->inEraser = false;

	this->extendedWarningDisplayd = false;

	this->verticalSpace = NULL;

	this->selection = NULL;

	this->textEditor = NULL;

	//this does not have to be deleted afterwards:
	//(we need it for undo commands)
	this->oldtext = NULL;

	this->search = NULL;

	this->eraser = new EraseHandler(xournal->getControl()->getUndoRedoHandler(), xournal->getControl()->getDocument(),
									this->page, xournal->getControl()->getToolHandler(), this);

	this->inputHandler = new InputHandler(this->xournal, this);
}

PageView::~PageView()
{
	XOJ_CHECK_TYPE(PageView);

	this->xournal->getControl()->getScheduler()->removePage(this);
	delete this->inputHandler;
	this->inputHandler = NULL;
	delete this->eraser;
	this->eraser = NULL;
	endText();
	deleteViewBuffer();

	for (Rectangle* rect : this->rerenderRects) delete rect;

	if (this->search) delete this->search;
	this->search = NULL;

	XOJ_RELEASE_TYPE(PageView);
}

void PageView::setIsVisible(bool visible)
{
	XOJ_CHECK_TYPE(PageView);

	if (visible)
	{
		this->lastVisibleTime = 0;
	}
	else if (this->lastVisibleTime <= 0)
	{
		GTimeVal val;
		g_get_current_time(&val);
		this->lastVisibleTime = val.tv_sec;
	}
}

int PageView::getLastVisibleTime()
{
	XOJ_CHECK_TYPE(PageView);

	if (this->crBuffer == NULL)
	{
		return -1;
	}

	return this->lastVisibleTime;
}

void PageView::deleteViewBuffer()
{
	XOJ_CHECK_TYPE(PageView);

	g_mutex_lock(&this->drawingMutex);
	if (this->crBuffer)
	{
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
	g_mutex_unlock(&this->drawingMutex);
}

bool PageView::containsPoint(int x, int y, bool local)
{
	XOJ_CHECK_TYPE(PageView);

	if (!local)
	{
		bool leftOk = this->layout.getLayoutAbsoluteX() <= x;
		bool rightOk = x <= this->layout.getLayoutAbsoluteX() + this->getDisplayWidth();
		bool topOk = this->layout.getLayoutAbsoluteY() <= y;
		bool bottomOk = y <= this->layout.getLayoutAbsoluteY() + this->getDisplayHeight();

		return leftOk && rightOk && topOk && bottomOk;
	}
	else
	{
		return x >= 0 && y >= 0 && x <= this->getWidth() && y <= this->getHeight();
	}
}

bool PageView::searchTextOnPage(string& text, int* occures, double* top)
{
	XOJ_CHECK_TYPE(PageView);

	if (this->search == NULL)
	{
		if (text.empty()) return true;

		int pNr = this->page->getPdfPageNr();
		XojPopplerPage* pdf = NULL;
		if (pNr != -1)
		{
			Document* doc = xournal->getControl()->getDocument();

			doc->lock();
			pdf = doc->getPdfPage(pNr);
			doc->unlock();
		}
		this->search = new SearchControl(page, pdf);
	}

	bool found = this->search->search(text, occures, top);

	repaintPage();

	return found;
}

void PageView::endText()
{
	XOJ_CHECK_TYPE(PageView);

	if (!this->textEditor)
	{
		return;
	}
	Text* txt = this->textEditor->getText();
	Layer* layer = this->page->getSelectedLayer();
	UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();

	// Text deleted
	if (txt->getText().empty())
	{
		// old element
		int pos = layer->indexOf(txt);
		if (pos != -1)
		{
			DeleteUndoAction* eraseDeleteUndoAction = new DeleteUndoAction(page, true);
			layer->removeElement(txt, false);
			eraseDeleteUndoAction->addElement(layer, txt, pos);
			undo->addUndoAction(eraseDeleteUndoAction);
		}
	}
	else
	{
		// new element
		if (layer->indexOf(txt) == -1)
		{
			undo->addUndoActionBefore(new InsertUndoAction(page, layer, txt), this->textEditor->getFirstUndoAction());
			layer->addElement(txt);
			this->textEditor->textCopyed();
		}
			// or if the file was saved and reopened
			// and/or if we click away from the text window
		else
		{
			// TextUndoAction does not work because the textEdit object is destroyed
			// after endText() so we need to instead copy the information between an
			// old and new element that we can push and pop to recover.
			undo->addUndoAction(new TextBoxUndoAction(page, layer, txt, this->oldtext));
		}
	}

	delete this->textEditor;
	this->textEditor = NULL;
	this->rerenderPage();
}

void PageView::startText(double x, double y)
{
	XOJ_CHECK_TYPE(PageView);

	this->xournal->endTextAllPages(this);

	if (this->textEditor == NULL)
	{
		// Is there already a textfield?
		Text* text = NULL;
		
		for (Element* e : *this->page->getSelectedLayer()->getElements())
		{
			if (e->getType() == ELEMENT_TEXT)
			{
				GdkRectangle matchRect = { gint(x - 10), gint(y - 10), 20, 20 };
				if (e->intersectsArea(&matchRect))
				{
					text = (Text*) e;
					break;
				}
			}
		}

		bool ownText = false;
		if (text == NULL)
		{
			ToolHandler* h = xournal->getControl()->getToolHandler();
			ownText = true;
			text = new Text();
			text->setX(x);
			text->setY(y);
			text->setColor(h->getColor());
			text->setFont(settings->getFont());
		}
		else
		{

			// We can try to add an undo action here. The initial text shows up in this
			// textEditor element.
			this->oldtext = text;
			// text = new Text(*oldtext);
			// need to clone the old text so that references still work properly.
			// cloning breaks things a little. do it manually
			text = new Text();
			text->setX(oldtext->getX());
			text->setY(oldtext->getY());
			text->setColor(oldtext->getColor());
			text->setFont(oldtext->getFont());
			text->setText(oldtext->getText());

			Layer* layer = this->page->getSelectedLayer();
			layer->removeElement(this->oldtext, false);
			layer->addElement(text);
			// perform the old swap onto the new text drawn.
		}

		this->textEditor = new TextEditor(this, xournal->getWidget(), text, ownText);
		if (!ownText)
		{
			this->textEditor->mousePressed(x - text->getX(), y - text->getY());
		}

		this->rerenderPage();
	}
	else
	{
		Text* text = this->textEditor->getText();
		GdkRectangle matchRect = {gint(x - 10), gint(y - 10), 20, 20};
		if (!text->intersectsArea(&matchRect))
		{
			endText();
		}
		else
		{
			this->textEditor->mousePressed(x - text->getX(), y - text->getY());
		}
	}
}

void PageView::selectObjectAt(double x, double y)
{
	XOJ_CHECK_TYPE(PageView);

	int selected = this->page->getSelectedLayerId();
	GdkRectangle matchRect = { gint(x - 10), gint(y - 10), 20, 20 };

	Stroke* strokeMatch = NULL;
	double gap = 1000000000;

	Element* elementMatch = NULL;

	// clear old selection anyway
	this->xournal->getControl()->clearSelection();

	for (Layer* l : *this->page->getLayers())
	{
		for (Element* e : *l->getElements())
		{
			if (e->intersectsArea(&matchRect))
			{
				if (e->getType() == ELEMENT_STROKE)
				{
					Stroke* s = (Stroke*) e;
					double tmpGap = 0;
					if ((s->intersects(x, y, 5, &tmpGap)) && (gap > tmpGap))
					{
						gap = tmpGap;
						strokeMatch = s;
					}
				}
				else
				{
					elementMatch = e;
				}
			}
		}

		selected--;
	}

	if (strokeMatch)
	{
		elementMatch = strokeMatch;
	}

	if (elementMatch)
	{
		xournal->setSelection(new EditSelection(xournal->getControl()->getUndoRedoHandler(), elementMatch, this, page));

		repaintPage();
	}
}

bool PageView::onButtonPressEvent(GtkWidget* widget, GdkEventButton* event)
{
	XOJ_CHECK_TYPE(PageView);

	if ((event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0)
	{
		return false; // not handled here
	}

	if (!this->selected)
	{
		xournal->getControl()->firePageSelected(this->page);
	}

	ToolHandler* h = xournal->getControl()->getToolHandler();

	double x = event->x;
	double y = event->y;

	if ((x < 0 || y < 0) && !extendedWarningDisplayd &&
		settings->isXinputEnabled())
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *xournal->getControl()->getWindow(),
												   GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE,
												   "%s", _C("There was a wrong input event, input is not working.\n"
															"Do you want to disable \"Extended Input\"?"));

		gtk_dialog_add_button(GTK_DIALOG(dialog), "Disable \"Extended Input\"", 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", 2);

		this->extendedWarningDisplayd = true;

		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->xournal->getControl()->getWindow()->getWindow()));
		if (gtk_dialog_run(GTK_DIALOG(dialog)) == 1)
		{
			settings->setXinputEnabled(false);
			xournal->updateXEvents();
		}
		gtk_widget_destroy(dialog);
		return true;
	}

	double zoom = xournal->getZoom();
	x /= zoom;
	y /= zoom;

	Cursor* cursor = xournal->getCursor();
	cursor->setMouseDown(true);

	if (h->getToolType() == TOOL_PEN)
	{
		this->inputHandler->startStroke(event, STROKE_TOOL_PEN, x, y);
	}
	else if (h->getToolType() == TOOL_HILIGHTER)
	{
		this->inputHandler->startStroke(event, STROKE_TOOL_HIGHLIGHTER, x, y);
	}
	else if (h->getToolType() == TOOL_ERASER)
	{
		if (h->getEraserType() == ERASER_TYPE_WHITEOUT)
		{
			this->inputHandler->startStroke(event, STROKE_TOOL_ERASER, x, y);
			this->inputHandler->getTmpStroke()->setColor(0xffffff); // White
		}
		else
		{
			this->eraser->erase(x, y);
			this->inEraser = true;
		}
	}
	else if (h->getToolType() == TOOL_VERTICAL_SPACE)
	{
		this->verticalSpace = new VerticalToolHandler(this, this->page, y, zoom);
	}
	/*
	else if (h->getToolType() == TOOL_DRAW_RECT || h->getToolType() == TOOL_DRAW_CIRCLE || h->getToolType() == TOOL_DRAW_ARROW)
	{
		if (h->getToolType() == TOOL_DRAW_RECT)
		{
		}
		else if (h->getToolType() == TOOL_DRAW_CIRCLE)
		{
		}
		else if (h->getToolType() == TOOL_DRAW_ARROW)
		{
		}
	}
	 */
	else if (h->getToolType() == TOOL_SELECT_RECT ||
			 h->getToolType() == TOOL_SELECT_REGION ||
			 h->getToolType() == TOOL_SELECT_OBJECT)
	{
		if (h->getToolType() == TOOL_SELECT_RECT)
		{
			if (this->selection)
			{
				delete this->selection;
				this->selection = NULL;
				repaintPage();
			}
			this->selection = new RectSelection(x, y, this);
		}
		else if (h->getToolType() == TOOL_SELECT_REGION)
		{
			if (this->selection)
			{
				delete this->selection;
				this->selection = NULL;
				repaintPage();
			}
			this->selection = new RegionSelect(x, y, this);
		}
		else if (h->getToolType() == TOOL_SELECT_OBJECT)
		{
			selectObjectAt(x, y);
		}
	}
	else if (h->getToolType() == TOOL_TEXT)
	{
		startText(x, y);
	}
	else if (h->getToolType() == TOOL_IMAGE)
	{
		ImageHandler imgHandler(xournal->getControl(), this);
		imgHandler.insertImage(x, y);
	}

	return true;
}

void PageView::resetShapeRecognizer()
{
	XOJ_CHECK_TYPE(PageView);

	this->inputHandler->resetShapeRecognizer();
}

bool PageView::onMotionNotifyEvent(GtkWidget* widget, GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(PageView);

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;

	ToolHandler* h = xournal->getControl()->getToolHandler();

	if (containsPoint(x, y, true) &&
		this->inputHandler->onMotionNotifyEvent(event))
	{
		//input	handler used this event
	}
	else if (this->selection)
	{
		this->selection->currentPos(x, y);
	}
	else if (this->verticalSpace)
	{
		this->verticalSpace->currentPos(x, y);
	}
	else if (this->textEditor)
	{
		Cursor* cursor = getXournal()->getCursor();
		cursor->setInvisible(false);

		Text* text = this->textEditor->getText();
		this->textEditor->mouseMoved(x - text->getX(), y - text->getY());
	}
	else if (h->getToolType() == TOOL_ERASER && h->getEraserType() != ERASER_TYPE_WHITEOUT && this->inEraser)
	{
		this->eraser->erase(x, y);
	}

	return false;
}

void PageView::translateEvent(GdkEvent* event, int xOffset, int yOffset)
{
	XOJ_CHECK_TYPE(PageView);

	double* x = NULL;
	double* y = NULL;

	if (event->type == GDK_MOTION_NOTIFY)
	{
		GdkEventMotion* ev = &event->motion;
		x = &ev->x;
		y = &ev->y;
	}
	else if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE)
	{
		GdkEventButton* ev = &event->button;
		x = &ev->x;
		y = &ev->y;
	}
	else
	{
		g_warning("PageView::translateEvent unknown event type: %i", event->type);
		return;
	}

	*x -= this->getX() - xOffset;
	*y -= this->getY() - yOffset;
}

bool PageView::onButtonReleaseEvent(GtkWidget* widget, GdkEventButton* event)
{
	XOJ_CHECK_TYPE(PageView);

	Control* control = xournal->getControl();

	this->inputHandler->onButtonReleaseEvent(event, this->page);

	if (this->inEraser)
	{
		this->inEraser = false;
		Document* doc = this->xournal->getControl()->getDocument();
		doc->lock();
		this->eraser->finalize();
		doc->unlock();
	}

	if (this->verticalSpace)
	{
		MoveUndoAction* undo = this->verticalSpace->finalize();
		delete this->verticalSpace;
		this->verticalSpace = NULL;
		control->getUndoRedoHandler()->addUndoAction(undo);
	}

	if (this->selection)
	{
		if (this->selection->finalize(this->page))
		{
			xournal->setSelection(new EditSelection(control->getUndoRedoHandler(), this->selection, this));
			delete this->selection;
			this->selection = NULL;
		}
		else
		{
			delete this->selection;
			this->selection = NULL;

			repaintPage();
		}

	}
	else if (this->textEditor)
	{
		this->textEditor->mouseReleased();
	}

	return false;
}

bool PageView::onKeyPressEvent(GdkEventKey* event)
{
	XOJ_CHECK_TYPE(PageView);

	// Esc leaves text edition
	if (event->keyval == GDK_Escape)
	{
		if (this->textEditor)
		{
			endText();
			return true;
		}
		else if (xournal->getSelection())
		{
			xournal->clearSelection();
			return true;
		}
		else
		{
			return false;
		}
	}

	if (this->textEditor && this->textEditor->onKeyPressEvent(event))
	{
		return true;
	}

	return false;
}

bool PageView::onKeyReleaseEvent(GdkEventKey* event)
{
	XOJ_CHECK_TYPE(PageView);

	if (this->textEditor && this->textEditor->onKeyReleaseEvent(event))
	{
		return true;
	}

	return false;
}

void PageView::rerenderPage()
{
	XOJ_CHECK_TYPE(PageView);

	this->rerenderComplete = true;
	this->xournal->getControl()->getScheduler()->addRerenderPage(this);
}

void PageView::repaintPage()
{
	XOJ_CHECK_TYPE(PageView);

	xournal->getRepaintHandler()->repaintPage(this);
}

void PageView::repaintArea(double x1, double y1, double x2, double y2)
{
	XOJ_CHECK_TYPE(PageView);

	double zoom = xournal->getZoom();
	xournal->getRepaintHandler()->repaintPageArea(this, x1 * zoom - 10, y1 * zoom - 10, x2 * zoom + 20, y2 * zoom + 20);
	//xournal->getRepaintHandler()->repaintPageArea(this, x1 * zoom - 10, y1 * zoom - 10, x2 * zoom + 90, y2 * zoom + 90);
}

Rectangle* PageView::rectOnWidget(double x, double y, double width, double height)
{
	XOJ_CHECK_TYPE(PageView);

	double zoom = xournal->getZoom();

	return new Rectangle(x * zoom - 10, y * zoom - 10, width * zoom + 20, height * zoom + 20);
}

void PageView::rerenderRect(double x, double y, double width, double heigth)
{
	XOJ_CHECK_TYPE(PageView);

	int rx = (int) MAX(x - 10, 0);
	int ry = (int) MAX(y - 10, 0);
	int rwidth = (int) (width + 20);
	int rheight = (int) (heigth + 20);

	addRerenderRect(rx, ry, rwidth, rheight);
}

void PageView::addRerenderRect(double x, double y, double width, double height)
{
	XOJ_CHECK_TYPE(PageView);

	if (this->rerenderComplete)
	{
		return;
	}

	Rectangle* rect = new Rectangle(x, y, width, height);

	Rectangle dest;

	g_mutex_lock(&this->repaintRectMutex);

	for (Rectangle* r : this->rerenderRects)
	{
		// its faster to redraw only one rect than repaint twice the same area
		// so loop through the rectangles to be redrawn, if new rectangle
		// intersects any of them, replace it by the union with the new one
		if (r->intersects(*rect))
		{
			r->add(*rect);

			delete rect;

			g_mutex_unlock(&this->repaintRectMutex);
			return;
		}
	}

	this->rerenderRects.push_back(rect);
	g_mutex_unlock(&this->repaintRectMutex);

	this->xournal->getControl()->getScheduler()->addRerenderPage(this);
}

void PageView::setSelected(bool selected)
{
	XOJ_CHECK_TYPE(PageView);

	this->selected = selected;

	if (selected)
	{
		this->xournal->requestFocus();
		this->xournal->getRepaintHandler()->repaintPageBorder(this);
	}
}

bool PageView::cut()
{
	XOJ_CHECK_TYPE(PageView);

	if (this->textEditor)
	{
		this->textEditor->cutToClipboard();
		return true;
	}
	return false;
}

bool PageView::copy()
{
	XOJ_CHECK_TYPE(PageView);

	if (this->textEditor)
	{
		this->textEditor->copyToCliboard();
		return true;
	}
	return false;
}

bool PageView::paste()
{
	XOJ_CHECK_TYPE(PageView);

	if (this->textEditor)
	{
		this->textEditor->pasteFromClipboard();
		return true;
	}
	return false;
}

bool PageView::actionDelete()
{
	XOJ_CHECK_TYPE(PageView);

	if (this->textEditor)
	{
		this->textEditor->deleteFromCursor(GTK_DELETE_CHARS, 1);
		return true;
	}
	return false;
}

bool PageView::paintPage(cairo_t* cr, GdkRectangle* rect)
{
	XOJ_CHECK_TYPE(PageView);

	static const char* txtLoading = _C("Loading...");

	double zoom = xournal->getZoom();

	g_mutex_lock(&this->drawingMutex);

	int dispWidth = getDisplayWidth();
	int dispHeight = getDisplayHeight();

	if (this->crBuffer == NULL)
	{
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dispWidth, dispHeight);
		cairo_t* cr2 = cairo_create(this->crBuffer);
		cairo_set_source_rgb(cr2, 1, 1, 1);
		cairo_rectangle(cr2, 0, 0, dispWidth, dispHeight);
		cairo_fill(cr2);

		cairo_scale(cr2, zoom, zoom);

		cairo_text_extents_t ex;
		cairo_set_source_rgb(cr2, 0.5, 0.5, 0.5);
		cairo_select_font_face(cr2, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr2, 32.0);
		cairo_text_extents(cr2, txtLoading, &ex);
		cairo_move_to(cr2, (page->getWidth() - ex.width) / 2 - ex.x_bearing,
						   (page->getHeight() - ex.height) / 2 - ex.y_bearing);
		cairo_show_text(cr2, txtLoading);

		cairo_destroy(cr2);
		rerenderPage();
	}

	cairo_save(cr);

	double width = cairo_image_surface_get_width(this->crBuffer);
	if (width != dispWidth)
	{
		double scale = ((double) dispWidth) / ((double) width);

		// Scale current image to fit the zoom level
		cairo_scale(cr, scale, scale);
		cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);

		cairo_set_source_surface(cr, this->crBuffer, 0, 0);

		rerenderPage();

		rect = NULL;
	}
	else
	{
		cairo_set_source_surface(cr, this->crBuffer, 0, 0);
	}

	if (rect)
	{
		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_fill(cr);

#ifdef DEBUG_SHOW_PAINT_BOUNDS
		cairo_set_source_rgb(cr, 1.0, 0.5, 1.0);
		cairo_set_line_width(cr, 1. / zoom);
		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_stroke(cr);
#endif
	}
	else
	{
		cairo_paint(cr);
	}

	cairo_restore(cr);

	// don't paint this with scale, because it needs a 1:1 zoom
	if (this->verticalSpace)
	{
		this->verticalSpace->paint(cr, rect, zoom);
	}

	cairo_scale(cr, zoom, zoom);

	if (this->textEditor)
	{
		this->textEditor->paint(cr, rect, zoom);
	}
	if (this->selection)
	{
		this->selection->paint(cr, rect, zoom);
	}

	if (this->search)
	{
		this->search->paint(cr, rect, zoom, getSelectionColor());
	}
	this->inputHandler->draw(cr, zoom);
	g_mutex_unlock(&this->drawingMutex);
	return true;
}

bool PageView::containsY(int y)
{
	XOJ_CHECK_TYPE(PageView);

	return (y >= this->getY() && y <= (this->getY() + this->getDisplayHeight()));
}

/**
 * GETTER / SETTER
 */

bool PageView::isSelected()
{
	XOJ_CHECK_TYPE(PageView);

	return selected;
}

int PageView::getBufferPixels()
{
	XOJ_CHECK_TYPE(PageView);

	if (crBuffer)
	{
		return cairo_image_surface_get_width(crBuffer) * cairo_image_surface_get_height(crBuffer);
	}
	return 0;
}

GdkColor PageView::getSelectionColor()
{
	XOJ_CHECK_TYPE(PageView);

	return this->xournal->getWidget()->style->base[GTK_STATE_SELECTED];
}

TextEditor* PageView::getTextEditor()
{
	XOJ_CHECK_TYPE(PageView);

	return textEditor;
}

int PageView::getX()
{
	XOJ_CHECK_TYPE(PageView);

	return this->layout.getLayoutAbsoluteX();
}

int PageView::getY()
{
	XOJ_CHECK_TYPE(PageView);

	return this->layout.getLayoutAbsoluteY();
}

PageRef PageView::getPage()
{
	XOJ_CHECK_TYPE(PageView);

	return page;
}

XournalView* PageView::getXournal()
{
	XOJ_CHECK_TYPE(PageView);

	return this->xournal;
}

double PageView::getHeight()
{
	XOJ_CHECK_TYPE(PageView);

	return this->page->getHeight();
}

double PageView::getWidth()
{
	XOJ_CHECK_TYPE(PageView);

	return this->page->getWidth();
}

int PageView::getDisplayWidth()
{
	XOJ_CHECK_TYPE(PageView);

	return this->page->getWidth() * this->xournal->getZoom();
}

int PageView::getDisplayHeight()
{
	XOJ_CHECK_TYPE(PageView);

	return this->page->getHeight() * this->xournal->getZoom();
}

TexImage* PageView::getSelectedTex()
{
	XOJ_CHECK_TYPE(PageView);

	EditSelection* theSelection = this->xournal->getSelection();
	if (!theSelection)
	{
		return NULL;
	}

	TexImage* texMatch = NULL;

	for (Element* e : *theSelection->getElements())
	{
		if (e->getType() == ELEMENT_TEXIMAGE)
		{
			texMatch = (TexImage*) e;
		}
	}
	return texMatch;

}

void PageView::rectChanged(Rectangle& rect)
{
	rerenderRect(rect.x, rect.y, rect.width, rect.height);
}

void PageView::rangeChanged(Range &range)
{
	rerenderRange(range);
}

void PageView::pageChanged()
{
	rerenderPage();
}

void PageView::elementChanged(Element* elem)
{
	rerenderElement(elem);
}
