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
#include "control/tools/ArrowHandler.h"
#include "control/tools/CoordinateSystemHandler.h"
#include "control/tools/CircleHandler.h"
#include "control/tools/EraseHandler.h"
#include "control/tools/ImageHandler.h"
#include "control/tools/InputHandler.h"
#include "control/tools/RectangleHandler.h"
#include "control/tools/RulerHandler.h"
#include "control/tools/Selection.h"
#include "control/tools/StrokeHandler.h"
#include "control/tools/VerticalToolHandler.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "model/PageRef.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "undo/DeleteUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/TextBoxUndoAction.h"
#include "view/TextView.h"
#include "widgets/XournalWidget.h"

#include "PageViewFindObjectHelper.h"

#include <config.h>
#include <config-debug.h>
#include <config-features.h>
#include <i18n.h>
#include <pixbuf-utils.h>
#include <Range.h>
#include <Rectangle.h>

#include <gdk/gdk.h>

#include <stdlib.h>
#include <math.h>

XojPageView::XojPageView(XournalView* xournal, PageRef page)
{
	XOJ_INIT_TYPE(XojPageView);

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

	this->verticalSpace = NULL;

	this->selection = NULL;

	this->textEditor = NULL;

	// this does not have to be deleted afterwards:
	// (we need it for undo commands)
	this->oldtext = NULL;

	this->search = NULL;

	this->eraser = new EraseHandler(xournal->getControl()->getUndoRedoHandler(), xournal->getControl()->getDocument(),
									this->page, xournal->getControl()->getToolHandler(), this);

	this->inputHandler = NULL;
}

XojPageView::~XojPageView()
{
	XOJ_CHECK_TYPE(XojPageView);

	// Unregister listener before destroying this handler
	this->unregisterListener();

	this->xournal->getControl()->getScheduler()->removePage(this);
	delete this->inputHandler;
	this->inputHandler = NULL;
	delete this->eraser;
	this->eraser = NULL;
	endText();
	deleteViewBuffer();

	for (Rectangle* rect : this->rerenderRects)
	{
		delete rect;
	}
	this->rerenderRects.clear();

	delete this->search;
	this->search = NULL;

	XOJ_RELEASE_TYPE(XojPageView);
}

void XojPageView::setIsVisible(bool visible)
{
	XOJ_CHECK_TYPE(XojPageView);

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

int XojPageView::getLastVisibleTime()
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->crBuffer == NULL)
	{
		return -1;
	}

	return this->lastVisibleTime;
}

void XojPageView::deleteViewBuffer()
{
	XOJ_CHECK_TYPE(XojPageView);

	g_mutex_lock(&this->drawingMutex);
	if (this->crBuffer)
	{
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
	g_mutex_unlock(&this->drawingMutex);
}

bool XojPageView::containsPoint(int x, int y, bool local)
{
	XOJ_CHECK_TYPE(XojPageView);

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

bool XojPageView::searchTextOnPage(string& text, int* occures, double* top)
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->search == NULL)
	{
		if (text.empty()) return true;

		int pNr = this->page->getPdfPageNr();
		XojPdfPageSPtr pdf = NULL;
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

void XojPageView::endText()
{
	XOJ_CHECK_TYPE(XojPageView);

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

void XojPageView::startText(double x, double y)
{
	XOJ_CHECK_TYPE(XojPageView);

	this->xournal->endTextAllPages(this);
	this->xournal->getControl()->getSearchBar()->showSearchBar(false);

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

bool XojPageView::onButtonPressEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(XojPageView);

	if ((pos.state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0)
	{
		return false; // not handled here
	}

	if (!this->selected)
	{
		xournal->getControl()->firePageSelected(this->page);
	}

	ToolHandler* h = xournal->getControl()->getToolHandler();

	double x = pos.x;
	double y = pos.y;

	if (x < 0 || y < 0)
	{
		return false;
	}

	double zoom = xournal->getZoom();
	x /= zoom;
	y /= zoom;

	Cursor* cursor = xournal->getCursor();
	cursor->setMouseDown(true);

	if (h->getToolType() == TOOL_PEN || h->getToolType() == TOOL_HILIGHTER ||
		(h->getToolType() == TOOL_ERASER && h->getEraserType() == ERASER_TYPE_WHITEOUT))
	{
		delete this->inputHandler;
		this->inputHandler = NULL;

		if (h->getDrawingType() == DRAWING_TYPE_LINE)
		{
			this->inputHandler = new RulerHandler(this->xournal, this, getPage());
		}
		else if (h->getDrawingType() == DRAWING_TYPE_RECTANGLE)
		{
			this->inputHandler = new RectangleHandler(this->xournal, this, getPage());
		}
		else if (h->getDrawingType() == DRAWING_TYPE_CIRCLE)
		{
			this->inputHandler = new CircleHandler(this->xournal, this, getPage());
		}
		else if (h->getDrawingType() == DRAWING_TYPE_ARROW)
		{
			this->inputHandler = new ArrowHandler(this->xournal, this, getPage());
		}
		else if (h->getDrawingType() == DRAWING_TYPE_COORDINATE_SYSTEM)
		{
			this->inputHandler = new CoordinateSystemHandler(this->xournal, this, getPage());
		}
		else
		{
			this->inputHandler = new StrokeHandler(this->xournal, this, getPage());
		}

		this->inputHandler->onButtonPressEvent(pos);
	}
	else if(h->getToolType() == TOOL_ERASER)
	{
		this->eraser->erase(x, y);
		this->inEraser = true;
	}
	else if (h->getToolType() == TOOL_VERTICAL_SPACE)
	{
		this->verticalSpace = new VerticalToolHandler(this, this->page, y, zoom);
	}
	else if (h->getToolType() == TOOL_SELECT_RECT ||
	         h->getToolType() == TOOL_SELECT_REGION ||
	         h->getToolType() == TOOL_PLAY_OBJECT ||
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
			SelectObject select(this);
			select.at(x, y);
		}
		else if (h->getToolType() == TOOL_PLAY_OBJECT)
		{
			PlayObject play(this);
			play.at(x, y);
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

void XojPageView::resetShapeRecognizer()
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->inputHandler != NULL)
	{
		this->inputHandler->resetShapeRecognizer();
	}
}

bool XojPageView::onMotionNotifyEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(XojPageView);

	double zoom = xournal->getZoom();
	double x = pos.x / zoom;
	double y = pos.y / zoom;

	ToolHandler* h = xournal->getControl()->getToolHandler();

	if (containsPoint(x, y, true) &&
		this->inputHandler &&
		this->inputHandler->onMotionNotifyEvent(pos))
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

bool XojPageView::onButtonReleaseEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(XojPageView);

	Control* control = xournal->getControl();

	if (this->inputHandler)
	{
		this->inputHandler->onButtonReleaseEvent(pos);
		delete this->inputHandler;
		this->inputHandler = NULL;
	}

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

bool XojPageView::onKeyPressEvent(GdkEventKey* event)
{
	XOJ_CHECK_TYPE(XojPageView);

	// Esc leaves text edition
	if (event->keyval == GDK_KEY_Escape)
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

bool XojPageView::onKeyReleaseEvent(GdkEventKey* event)
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->textEditor && this->textEditor->onKeyReleaseEvent(event))
	{
		return true;
	}

	return false;
}

void XojPageView::rerenderPage()
{
	XOJ_CHECK_TYPE(XojPageView);

	this->rerenderComplete = true;
	this->xournal->getControl()->getScheduler()->addRerenderPage(this);
}

void XojPageView::repaintPage()
{
	XOJ_CHECK_TYPE(XojPageView);

	xournal->getRepaintHandler()->repaintPage(this);
}

void XojPageView::repaintArea(double x1, double y1, double x2, double y2)
{
	XOJ_CHECK_TYPE(XojPageView);

	double zoom = xournal->getZoom();
	xournal->getRepaintHandler()->repaintPageArea(this, x1 * zoom - 10, y1 * zoom - 10, x2 * zoom + 20, y2 * zoom + 20);
}

void XojPageView::rerenderRect(double x, double y, double width, double height)
{
	XOJ_CHECK_TYPE(XojPageView);

	int rx = (int) MAX(x - 10, 0);
	int ry = (int) MAX(y - 10, 0);
	int rwidth = (int) (width + 20);
	int rheight = (int) (height + 20);

	addRerenderRect(rx, ry, rwidth, rheight);
}

void XojPageView::addRerenderRect(double x, double y, double width, double height)
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->rerenderComplete)
	{
		return;
	}

	Rectangle* rect = new Rectangle(x, y, width, height);

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

void XojPageView::setSelected(bool selected)
{
	XOJ_CHECK_TYPE(XojPageView);

	this->selected = selected;

	if (selected)
	{
		this->xournal->requestFocus();
		this->xournal->getRepaintHandler()->repaintPageBorder(this);
	}
}

bool XojPageView::cut()
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->textEditor)
	{
		this->textEditor->cutToClipboard();
		return true;
	}
	return false;
}

bool XojPageView::copy()
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->textEditor)
	{
		this->textEditor->copyToCliboard();
		return true;
	}
	return false;
}

bool XojPageView::paste()
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->textEditor)
	{
		this->textEditor->pasteFromClipboard();
		return true;
	}
	return false;
}

bool XojPageView::actionDelete()
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->textEditor)
	{
		this->textEditor->deleteFromCursor(GTK_DELETE_CHARS, 1);
		return true;
	}
	return false;
}

void XojPageView::drawLoadingPage(cairo_t* cr)
{
	XOJ_CHECK_TYPE(XojPageView);

	static const string txtLoading = _("Loading...");

	double zoom = xournal->getZoom();
	int dispWidth = getDisplayWidth();
	int dispHeight = getDisplayHeight();

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, 0, 0, dispWidth, dispHeight);
	cairo_fill(cr);

	cairo_scale(cr, zoom, zoom);

	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 32.0);
	cairo_text_extents_t ex;
	cairo_text_extents(cr, txtLoading.c_str(), &ex);
	cairo_move_to(cr, (page->getWidth() - ex.width) / 2 - ex.x_bearing,
					  (page->getHeight() - ex.height) / 2 - ex.y_bearing);
	cairo_show_text(cr, txtLoading.c_str());

	rerenderPage();
}

/**
 * Does the painting, called in synchronized block
 */
void XojPageView::paintPageSync(cairo_t* cr, GdkRectangle* rect)
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->crBuffer == NULL)
	{
		drawLoadingPage(cr);
		return;
	}

	double zoom = xournal->getZoom();
	int dispWidth = getDisplayWidth();

	cairo_save(cr);

	double width = cairo_image_surface_get_width(this->crBuffer);

	bool rerender = true;
	if (width / xournal->getDpiScaleFactor() == dispWidth)
	{
		rerender = false;
	}

	if (width != dispWidth)
	{
		double scale = ((double) dispWidth) / ((double) width);

		// Scale current image to fit the zoom level
		cairo_scale(cr, scale, scale);
		cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);

		cairo_set_source_surface(cr, this->crBuffer, 0, 0);

		if (rerender)
		{
			rerenderPage();
		}

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

	if (this->textEditor)
	{
		cairo_scale(cr, zoom, zoom);
		this->textEditor->paint(cr, rect, zoom);
	}
	if (this->selection)
	{
		cairo_scale(cr, zoom, zoom);
		this->selection->paint(cr, rect, zoom);
	}

	if (this->search)
	{
		cairo_scale(cr, zoom, zoom);
		this->search->paint(cr, rect, zoom, getSelectionColor());
	}

	if (this->inputHandler)
	{
		int dpiScaleFactor = xournal->getDpiScaleFactor();
		cairo_scale(cr, 1.0 / dpiScaleFactor, 1.0 / dpiScaleFactor);
		this->inputHandler->draw(cr);
	}
}

bool XojPageView::paintPage(cairo_t* cr, GdkRectangle* rect)
{
	XOJ_CHECK_TYPE(XojPageView);

	g_mutex_lock(&this->drawingMutex);

	paintPageSync(cr, rect);

	g_mutex_unlock(&this->drawingMutex);
	return true;
}

bool XojPageView::containsY(int y)
{
	XOJ_CHECK_TYPE(XojPageView);

	return (y >= this->getY() && y <= (this->getY() + this->getDisplayHeight()));
}

/**
 * GETTER / SETTER
 */

bool XojPageView::isSelected()
{
	XOJ_CHECK_TYPE(XojPageView);

	return selected;
}

int XojPageView::getBufferPixels()
{
	XOJ_CHECK_TYPE(XojPageView);

	if (crBuffer)
	{
		return cairo_image_surface_get_width(crBuffer) * cairo_image_surface_get_height(crBuffer);
	}
	return 0;
}

GtkColorWrapper XojPageView::getSelectionColor()
{
	XOJ_CHECK_TYPE(XojPageView);
	return settings->getSelectionColor();
}

TextEditor* XojPageView::getTextEditor()
{
	XOJ_CHECK_TYPE(XojPageView);

	return textEditor;
}

int XojPageView::getX() const
{
	XOJ_CHECK_TYPE(XojPageView);

	return this->layout.getLayoutAbsoluteX();
}

int XojPageView::getY() const
{
	XOJ_CHECK_TYPE(XojPageView);

	return this->layout.getLayoutAbsoluteY();
}

PageRef XojPageView::getPage()
{
	XOJ_CHECK_TYPE(XojPageView);

	return page;
}

XournalView* XojPageView::getXournal()
{
	XOJ_CHECK_TYPE(XojPageView);

	return this->xournal;
}

double XojPageView::getHeight() const
{
	XOJ_CHECK_TYPE(XojPageView);

	return this->page->getHeight();
}

double XojPageView::getWidth() const
{
	XOJ_CHECK_TYPE(XojPageView);

	return this->page->getWidth();
}

int XojPageView::getDisplayWidth() const
{
	XOJ_CHECK_TYPE(XojPageView);

	return this->page->getWidth() * this->xournal->getZoom();
}

int XojPageView::getDisplayHeight() const
{
	XOJ_CHECK_TYPE(XojPageView);

	return this->page->getHeight() * this->xournal->getZoom();
}

TexImage* XojPageView::getSelectedTex()
{
	XOJ_CHECK_TYPE(XojPageView);

	EditSelection* theSelection = this->xournal->getSelection();
	if (!theSelection)
	{
		return NULL;
	}

	for (Element* e : *theSelection->getElements())
	{
		if (e->getType() == ELEMENT_TEXIMAGE)
		{
			return (TexImage*) e;
		}
	}
	return NULL;
}

Text* XojPageView::getSelectedText()
{
	XOJ_CHECK_TYPE(XojPageView);

	EditSelection* theSelection = this->xournal->getSelection();
	if (!theSelection)
	{
		return NULL;
	}

	for (Element* e : *theSelection->getElements())
	{
		if (e->getType() == ELEMENT_TEXT)
		{
			return (Text*) e;
		}
	}
	return NULL;
}

Rectangle XojPageView::getRect()
{
	XOJ_CHECK_TYPE(XojPageView);

	return Rectangle(getX(), getY(), getDisplayWidth(), getDisplayHeight());
}

void XojPageView::rectChanged(Rectangle& rect)
{
	XOJ_CHECK_TYPE(XojPageView);

	rerenderRect(rect.x, rect.y, rect.width, rect.height);
}

void XojPageView::rangeChanged(Range &range)
{
	XOJ_CHECK_TYPE(XojPageView);

	rerenderRange(range);
}

void XojPageView::pageChanged()
{
	XOJ_CHECK_TYPE(XojPageView);

	rerenderPage();
}

void XojPageView::elementChanged(Element* elem)
{
	XOJ_CHECK_TYPE(XojPageView);

	if (this->inputHandler && elem == this->inputHandler->getStroke())
	{
		g_mutex_lock(&this->drawingMutex);

		cairo_t* cr = cairo_create(this->crBuffer);

		this->inputHandler->draw(cr);

		cairo_destroy(cr);

		g_mutex_unlock(&this->drawingMutex);
	}
	else
	{
		rerenderElement(elem);
	}
}
