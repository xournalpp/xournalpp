#include "EditSelectionContents.h"

#include "Selection.h"

#include "control/Control.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "undo/ColorUndoAction.h"
#include "undo/DeleteUndoAction.h"
#include "undo/FontUndoAction.h"
#include "undo/FillUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/MoveUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "undo/SizeUndoAction.h"
#include "undo/ScaleUndoAction.h"
#include "undo/RotateUndoAction.h"
#include "view/DocumentView.h"

#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>

#include <cmath>

EditSelectionContents::EditSelectionContents(double x, double y, double width, double height,
											 PageRef sourcePage, Layer* sourceLayer, XojPageView* sourceView)
{
	XOJ_INIT_TYPE(EditSelectionContents);

	this->crBuffer = NULL;

	this->rescaleId = 0;

	this->lastWidth = this->originalWidth = width;
	this->lastHeight = this->originalHeight = height;
	this->relativeX = -9999999999;
	this->relativeY = -9999999999;
	this->rotation = 0;

	this->lastX = this->originalX = x;
	this->lastY = this->originalY = y;

	this->sourcePage = sourcePage;
	this->sourceLayer = sourceLayer;
	this->sourceView = sourceView;
}

EditSelectionContents::~EditSelectionContents()
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	if (this->rescaleId)
	{
		g_source_remove(this->rescaleId);
		this->rescaleId = 0;
	}

	deleteViewBuffer();

	XOJ_RELEASE_TYPE(EditSelectionContents);
}

/**
 * Add an element to the this selection
 */
void EditSelectionContents::addElement(Element* e)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	this->selected.push_back(e);
}

/**
 * Returns all containig elements of this selections
 */
vector<Element*>* EditSelectionContents::getElements()
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	return &this->selected;
}

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or NULL if nothing is done)
 */
UndoAction* EditSelectionContents::setSize(ToolSize size,
										   const double* thicknessPen,
										   const double* thicknessHilighter,
										   const double* thicknessEraser)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	SizeUndoAction* undo = new SizeUndoAction(this->sourcePage, this->sourceLayer);

	bool found = false;

	for (Element* e : this->selected)
	{
		if (e->getType() == ELEMENT_STROKE)
		{
			Stroke* s = (Stroke*) e;
			StrokeTool tool = s->getToolType();

			double originalWidth = s->getWidth();

			int pointCount = s->getPointCount();
			vector<double> originalPressure = SizeUndoAction::getPressure(s);

			if (tool == STROKE_TOOL_PEN)
			{
				s->setWidth(thicknessPen[size]);
			}
			else if (tool == STROKE_TOOL_HIGHLIGHTER)
			{
				s->setWidth(thicknessHilighter[size]);
			}
			else if (tool == STROKE_TOOL_ERASER)
			{
				s->setWidth(thicknessEraser[size]);
			}

			// scale the stroke
			double factor = s->getWidth() / originalWidth;
			s->scalePressure(factor);

			// save the new pressure
			vector<double> newPressure = SizeUndoAction::getPressure(s);

			undo->addStroke(s, originalWidth, s->getWidth(), originalPressure, newPressure, pointCount);
			found = true;
		}
	}

	if (found)
	{
		this->deleteViewBuffer();
		this->sourceView->getXournal()->repaintSelection();

		return undo;
	}
	else
	{
		delete undo;
		return NULL;
	}
}

/**
 * Fills the stroke, return an undo action
 * (Or NULL if nothing done, e.g. because there is only an image)
 */
UndoAction* EditSelectionContents::setFill(int alphaPen, int alphaHighligther)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	FillUndoAction* undo = new FillUndoAction(this->sourcePage, this->sourceLayer);

	bool found = false;

	for (Element* e : this->selected)
	{
		if (e->getType() == ELEMENT_STROKE)
		{
			Stroke* s = (Stroke*) e;
			StrokeTool tool = s->getToolType();
			int newFill = 128;

			if (tool == STROKE_TOOL_PEN)
			{
				newFill = alphaPen;
			}
			else if (tool == STROKE_TOOL_HIGHLIGHTER)
			{
				newFill = alphaHighligther;
			}
			else
			{
				continue;
			}

			if (newFill == s->getFill())
			{
				continue;
			}

			bool originalFill = s->getFill();
			s->setFill(newFill);

			undo->addStroke(s, originalFill, newFill);
			found = true;
		}
	}

	if (found)
	{
		this->deleteViewBuffer();
		this->sourceView->getXournal()->repaintSelection();

		return undo;
	}
	else
	{
		delete undo;
		return NULL;
	}
}

/**
 * Sets the font of all containing text elements, return an undo action
 * (or NULL if there are no Text elements)
 */
UndoAction* EditSelectionContents::setFont(XojFont& font)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	double x1 = 0.0 / 0.0;
	double x2 = 0.0 / 0.0;
	double y1 = 0.0 / 0.0;
	double y2 = 0.0 / 0.0;

	FontUndoAction* undo = new FontUndoAction(this->sourcePage, this->sourceLayer);

	for (Element* e : this->selected)
	{
		if (e->getType() == ELEMENT_TEXT)
		{
			Text* t = (Text*) e;
			undo->addStroke(t, t->getFont(), font);

			if (std::isnan(x1))
			{
				x1 = t->getX();
				y1 = t->getY();
				x2 = t->getX() + t->getElementWidth();
				y2 = t->getY() + t->getElementHeight();
			}
			else
			{
				// size with old font
				x1 = MIN(x1, t->getX());
				y1 = MIN(y1, t->getY());

				x2 = MAX(x2, t->getX() + t->getElementWidth());
				y2 = MAX(y2, t->getY() + t->getElementHeight());
			}

			t->setFont(font);

			// size with new font
			x1 = MIN(x1, t->getX());
			y1 = MIN(y1, t->getY());

			x2 = MAX(x2, t->getX() + t->getElementWidth());
			y2 = MAX(y2, t->getY() + t->getElementHeight());
		}
	}

	if (!std::isnan(x1))
	{
		this->deleteViewBuffer();
		this->sourceView->getXournal()->repaintSelection();
		return undo;
	}
	delete undo;
	return NULL;
}

/**
 * Set the color of all elements, return an undo action
 * (Or NULL if nothing done, e.g. because there is only an image)
 */
UndoAction* EditSelectionContents::setColor(int color)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	ColorUndoAction* undo = new ColorUndoAction(this->sourcePage, this->sourceLayer);

	bool found = false;

	for (Element* e : this->selected)
	{
		if (e->getType() == ELEMENT_TEXT || e->getType() == ELEMENT_STROKE)
		{
			int lastColor = e->getColor();
			e->setColor(color);
			undo->addStroke(e, lastColor, e->getColor());

			found = true;
		}
	}

	if (found)
	{
		this->deleteViewBuffer();
		this->sourceView->getXournal()->repaintSelection();

		return undo;
	}
	else
	{
		delete undo;
		return NULL;
	}

	return NULL;
}

/**
 * Fills the undo item if the selection is deleted
 * the selection is cleared after
 */
void EditSelectionContents::fillUndoItem(DeleteUndoAction* undo)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	Layer* layer = this->sourceLayer;

	// Always insert the elements on top
	// Because the elements are already removed
	// and owned by the selection, therefore the layer
	// doesn't know the index anymore
	int index = layer->getElements()->size();
	for (Element* e : this->selected)
	{
		undo->addElement(layer, e, index);
	}

	this->selected.clear();
}

/**
 * Callback to redrawing the buffer asynchron
 */
bool EditSelectionContents::repaintSelection(EditSelectionContents* selection)
{
	XOJ_CHECK_TYPE_OBJ(selection, EditSelectionContents);

	// delete the selection buffer, force a redraw
	selection->deleteViewBuffer();
	selection->sourceView->getXournal()->repaintSelection();
	selection->rescaleId = 0;

	return false;
}

/**
 * Delete our internal View buffer,
 * it will be recreated when the selection is painted next time
 */
void EditSelectionContents::deleteViewBuffer()
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	if (this->crBuffer)
	{
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
}

/**
 * Gets the original width of the contents
 */
double EditSelectionContents::getOriginalWidth()
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	return this->originalWidth;
}

/**
 * Gets the original height of the contents
 */
double EditSelectionContents::getOriginalHeight()
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	return this->originalHeight;
}

/**
 * The contents of the selection
 */
void EditSelectionContents::finalizeSelection(double x, double y, double width, double height, bool aspectRatio,
											  Layer* layer, PageRef targetPage, XojPageView* targetView, UndoRedoHandler* undo)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	double fx = width / this->originalWidth;
	double fy = height / this->originalHeight;

	if (aspectRatio)
	{
		double f = (fx + fy) / 2;
		fx = f;
		fy = f;
	}
	bool scale = (width != this->originalWidth || height != this->originalHeight);
	bool rotate = (std::abs(this->rotation) > __DBL_EPSILON__);

	double mx = x - this->originalX;
	double my = y - this->originalY;

	bool move = mx != 0 || my != 0;

	for (Element* e : this->selected)
	{
		if (move)
		{
			e->move(mx, my);
		}
		if (scale)
		{
			e->scale(x, y, fx, fy);		
		}
		if (rotate)
		{
			e->rotate(x, y, this->lastWidth / 2, this->lastHeight / 2, this->rotation);
		}
		layer->addElement(e);
	}

}

double EditSelectionContents::getOriginalX()
{
	return this->originalX;
}

double EditSelectionContents::getOriginalY()
{
	return this->originalY;
}

XojPageView* EditSelectionContents::getSourceView()
{
	return this->sourceView;
}


void EditSelectionContents::updateContent(double x, double y, double rotation, double width, double height, bool aspectRatio,
										  Layer* layer, PageRef targetPage, XojPageView* targetView,
										  UndoRedoHandler* undo, CursorSelectionType type)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	double mx = x - this->lastX;
	double my = y - this->lastY;
	this->rotation = rotation;

	double fx = width / this->lastWidth;
	double fy = height / this->lastHeight;

	if (aspectRatio)
	{
		double f = (fx + fy) / 2;
		fx = f;
		fy = f;
	}

	bool scale = (width != this->lastWidth || height != this->lastHeight);
	bool rotate = (std::abs(this->rotation) > __DBL_EPSILON__);

	if (type == CURSOR_SELECTION_MOVE)
	{
		MoveUndoAction* moveUndo = new MoveUndoAction(this->sourceLayer, this->sourcePage, &this->selected,
													  mx, my, layer, targetPage);

		undo->addUndoAction(moveUndo);

	}
	else if (rotate)
	{
		RotateUndoAction* rotateUndo = new RotateUndoAction(this->sourcePage, &this->selected, x,
															y, width / 2, height / 2, this->rotation);
		undo->addUndoAction(rotateUndo);
		this->rotation = 0;	// reset rotation for next usage
	}
	if (scale)
	{
		// The coordinates which are the center of the scaling
		// operation. Their coordinates depend on the scaling
		// operation performed
		double px = this->lastX;
		double py = this->lastY;

		switch (type)
		{
		case CURSOR_SELECTION_TOP_LEFT:
		case CURSOR_SELECTION_BOTTOM_LEFT:
		case CURSOR_SELECTION_LEFT:
			px = (this->lastWidth + this->lastX);
			break;
		default:
			break;
		}

		switch (type)
		{
		case CURSOR_SELECTION_TOP_LEFT:
		case CURSOR_SELECTION_TOP_RIGHT:	
		case CURSOR_SELECTION_TOP:
			py = (this->lastHeight + this->lastY);
			break;
		default:
			break;
		}

		ScaleUndoAction* scaleUndo = new ScaleUndoAction(this->sourcePage, &this->selected, px, py, fx, fy);
		undo->addUndoAction(scaleUndo);
	}

	this->lastX = x;
	this->lastY = y;

	this->lastWidth = width;
	this->lastHeight = height;
}

/**
 * paints the selection
 */
void EditSelectionContents::paint(cairo_t* cr, double x, double y, double rotation, double width, double height, double zoom)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	double fx = width / this->originalWidth;
	double fy = height / this->originalHeight;
		
	if (this->relativeX == -9999999999)
	{
		this->relativeX = x;
		this->relativeY = y;
	}

	if (std::abs(rotation) > __DBL_EPSILON__)
	{
		this->rotation = rotation;
	}

	if (this->crBuffer == NULL)
	{
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width * zoom, height * zoom);
		cairo_t* cr2 = cairo_create(this->crBuffer);

		int dx = (int) (this->relativeX * zoom);
		int dy = (int) (this->relativeY * zoom);

		cairo_scale(cr2, fx, fy);
		cairo_translate(cr2, -dx, -dy);
		cairo_scale(cr2, zoom, zoom);
		DocumentView view;
		view.drawSelection(cr2, this);

		cairo_destroy(cr2);
	}

	cairo_save(cr);

	int wImg = cairo_image_surface_get_width(this->crBuffer);
	int hImg = cairo_image_surface_get_height(this->crBuffer);

	int wTarget = (int) (width * zoom);
	int hTarget = (int) (height * zoom);

	double sx = (double) wTarget / wImg;
	double sy = (double) hTarget / hImg;

	if (wTarget != wImg || hTarget != hImg || std::abs(rotation) > __DBL_EPSILON__)
	{
		if (!this->rescaleId)
		{
			this->rescaleId = g_idle_add((GSourceFunc) repaintSelection, this);
		}
		cairo_scale(cr, sx, sy);
	}

	double dx = (int) (x * zoom / sx);
	double dy = (int) (y * zoom / sy);

	cairo_set_source_surface(cr, this->crBuffer, dx, dy);
	cairo_paint(cr);

	cairo_restore(cr);
}

UndoAction* EditSelectionContents::copySelection(PageRef page, XojPageView *view, double x, double y)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	Layer* layer = page->getSelectedLayer();

	vector<Element*> new_elems;

	for (Element* e : *getElements())
	{
		Element* ec = e->clone();

		ec->move(x - this->originalX, y - this->originalY);

		layer->addElement(ec);
		new_elems.push_back(ec);
	}

	view->rerenderPage();

	return new InsertsUndoAction(page, layer, new_elems);
}

void EditSelectionContents::serialize(ObjectOutputStream& out)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	out.writeObject("EditSelectionContents");

	out.writeDouble(this->originalWidth);
	out.writeDouble(this->originalHeight);

	out.writeDouble(this->originalX);
	out.writeDouble(this->originalY);

	out.writeDouble(this->relativeX);
	out.writeDouble(this->relativeY);

	out.endObject();
}

void EditSelectionContents::readSerialized(ObjectInputStream& in)
{
	XOJ_CHECK_TYPE(EditSelectionContents);

	in.readObject("EditSelectionContents");

	this->originalWidth = in.readDouble();
	this->originalHeight = in.readDouble();

	this->originalX = in.readDouble();
	this->originalY = in.readDouble();

	this->relativeX = in.readDouble();
	this->relativeY = in.readDouble();

	in.endObject();
}
