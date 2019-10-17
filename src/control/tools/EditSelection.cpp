#include "EditSelection.h"

#include "EditSelectionContents.h"
#include "Selection.h"

#include "control/Control.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/Element.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "undo/ColorUndoAction.h"
#include "undo/FontUndoAction.h"
#include "undo/SizeUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "util/GtkColorWrapper.h"
#include "gui/Layout.h"

#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>

#include <cmath>


#define MINPIXSIZE 5	// smallest can scale down to, in pixels.

EditSelection::EditSelection(UndoRedoHandler* undo, PageRef page, XojPageView* view)
{
	this->x = 0;
	this->y = 0;
	this->rotation = 0;
	this->width = 0;
	this->height = 0;

	contstruct(undo, view, page);
}

EditSelection::EditSelection(UndoRedoHandler* undo, Selection* selection, XojPageView* view)
{
	calcSizeFromElements(selection->selectedElements);

	contstruct(undo, view, view->getPage());

	for (Element* e : selection->selectedElements)
	{
		this->sourceLayer->removeElement(e, false);
		addElement(e);
	}

	view->rerenderPage();
}

EditSelection::EditSelection(UndoRedoHandler* undo, Element* e, XojPageView* view, PageRef page)
{
	this->x = e->getX();
	this->y = e->getY();
	this->width = e->getElementWidth();
	this->height = e->getElementHeight();

	contstruct(undo, view, page);

	addElement(e);
	this->sourceLayer->removeElement(e, false);

	view->rerenderElement(e);
}

EditSelection::EditSelection(UndoRedoHandler* undo, vector<Element*> elements, XojPageView* view, PageRef page)
{
	calcSizeFromElements(elements);

	contstruct(undo, view, page);

	for (Element* e : elements)
	{
		addElement(e);
		this->sourceLayer->removeElement(e, false);
	}

	view->rerenderPage();
}

void EditSelection::calcSizeFromElements(vector<Element*> elements)
{
	if (elements.empty())
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		return;
	}

	Element* first = elements.front();
	Range range(first->getX(), first->getY());

	for (Element* e : elements)
	{
		range.addPoint(e->getX(), e->getY());
		range.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
	}

	x = range.getX() - 3;
	y = range.getY() - 3;
	width = range.getWidth() + 6;
	height = range.getHeight() + 6;
}

/**
 * Our internal constructor
 */
void EditSelection::contstruct(UndoRedoHandler* undo, XojPageView* view, PageRef sourcePage)
{
	this->view = view;
	this->undo = undo;
	this->sourcePage = sourcePage;
	this->sourceLayer = this->sourcePage->getSelectedLayer();

	this->aspectRatio = false;

	this->relMousePosX = 0;
	this->relMousePosY = 0;
	this->mouseDownType = CURSOR_SELECTION_NONE;
	
	int dpi = this->view->getXournal()->getControl()->getSettings()->getDisplayDpi();
	this->btnWidth = std::max(10,dpi/8);

	this->contents = new EditSelectionContents(this->x, this->y, this->width, this->height,
											   this->sourcePage, this->sourceLayer, this->view);
	
	cairo_matrix_init_identity(&this->cmatrix);
	
}

EditSelection::~EditSelection()
{
	finalizeSelection();

	this->sourcePage = nullptr;
	this->sourceLayer = nullptr;

	delete this->contents;
	this->contents = nullptr;

	this->view = nullptr;
	this->undo = nullptr;
}

/**
 * Finishes all pending changes, move the elements, scale the elements and add
 * them to new layer if any or to the old if no new layer
 */
void EditSelection::finalizeSelection()
{
	XojPageView* v = getPageViewUnderCursor();
	if (v == nullptr)
	{	// Not on any page - move back to original page and position
		this->x = this->contents->getOriginalX();
		this->y = this->contents->getOriginalY();
		v = this->contents->getSourceView();
	}
	

	this->view = v;

	PageRef page = this->view->getPage();
	Layer* layer = page->getSelectedLayer();
	this->contents->finalizeSelection(this->x, this->y, this->width, this->height,
										this->aspectRatio, layer, page, this->view, this->undo);

	
	//Calculate new clip region delta due to rotation:
    double addW =std::abs(this->width * cos(this->rotation)) + std::abs(this->height * sin(this->rotation)) - this->width ;
	double addH = std::abs(this->width * sin(this->rotation)) + std::abs(this->height * cos(this->rotation)) - this->height;


	this->view->rerenderRect(this->x - addW/2.0, this->y-addH/2.0, this->width+addW, this->height+addH);

	// This is needed if the selection not was 100% on a page
	this->view->getXournal()->repaintSelection(true);

}

/**
 * get the X coordinate relative to the provided view (getView())
 * in document coordinates
 */
double EditSelection::getXOnView()
{
	return this->x;
}

/**
 * get the Y coordinate relative to the provided view (getView())
 * in document coordinates
 */
double EditSelection::getYOnView()
{
	return this->y;
}

double EditSelection::getOriginalXOnView()
{
	return this->contents->getOriginalX();
}

double EditSelection::getOriginalYOnView()
{
	return this->contents->getOriginalY();
}

/**
 * get the width in document coordinates (multiple with zoom)
 */
double EditSelection::getWidth()
{
	return this->width;
}

/**
 * get the height in document coordinates (multiple with zoom)
 */
double EditSelection::getHeight()
{
	return this->height;
}

/**
 * Get the source page (where the selection was done)
 */
PageRef EditSelection::getSourcePage()
{
	return this->sourcePage;
}

/**
 * Get the source layer (form where the Elements come)
 */
Layer* EditSelection::getSourceLayer()
{
	return this->sourceLayer;
}

/**
 * Get the X coordinate in View coordinates (absolute)
 */
int EditSelection::getXOnViewAbsolute()
{
	double zoom = view->getXournal()->getZoom();
	return this->view->getX() + this->getXOnView() * zoom;
}

/**
 * Get the Y coordinate in View coordinates (absolute)
 */
int EditSelection::getYOnViewAbsolute()
{
	double zoom = view->getXournal()->getZoom();
	return this->view->getY() + this->getYOnView() * zoom;
}

/**
 * Get the width in View coordinates
 */
int EditSelection::getViewWidth()
{
	double zoom = view->getXournal()->getZoom();
	return this->width * zoom;
}

/**
 * Get the height in View coordinates
 */
int EditSelection::getViewHeight()
{
	double zoom = view->getXournal()->getZoom();
	return this->height * zoom;
}

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or nullptr if nothing is done)
 */
UndoAction* EditSelection::setSize(ToolSize size,
								   const double* thicknessPen,
								   const double* thicknessHilighter,
								   const double* thicknessEraser)
{
	return this->contents->setSize(size, thicknessPen, thicknessHilighter, thicknessEraser);
}

/**
 * Fills the stroke, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
UndoAction* EditSelection::setFill(int alphaPen, int alphaHighligther)
{
	return this->contents->setFill(alphaPen, alphaHighligther);
}

/**
 * Set the color of all elements, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
UndoAction* EditSelection::setColor(int color)
{
	return this->contents->setColor(color);
}

/**
 * Sets the font of all containing text elements, return an undo action
 * (or nullptr if there are no Text elements)
 */
UndoAction* EditSelection::setFont(XojFont& font)
{
	return this->contents->setFont(font);
}

/**
 * Fills de undo item if the selection is deleted
 * the selection is cleared after
 */
void EditSelection::fillUndoItem(DeleteUndoAction* undo)
{
	this->contents->fillUndoItem(undo);
}

/**
 * Add an element to the this selection
 */
void EditSelection::addElement(Element* e)
{
	this->contents->addElement(e);

	if (e->rescaleOnlyAspectRatio())
	{
		this->aspectRatio = true;
	}

	if (e->getType() != ELEMENT_STROKE)
	{
		// Currently only stroke supports rotation
		supportRotation = false;
	}
}

/**
 * Returns all containig elements of this selections
 */
vector<Element*>* EditSelection::getElements()
{
	return this->contents->getElements();
}

/**
 * Finish the current movement
 * (should be called in the mouse-button-released event handler)
 */
void EditSelection::mouseUp()
{
	if (this->mouseDownType == CURSOR_SELECTION_DELETE )
	{
		this->view->getXournal()->deleteSelection();
		return;
		
	}
	else
	{
		PageRef page = this->view->getPage();
		Layer* layer = page->getSelectedLayer();

		snapRotation();

		this->contents->updateContent(this->x, this->y, this->rotation, this->width, this->height, this->aspectRatio,
									layer, page, this->view, this->undo, this->mouseDownType);
	}
	this->mouseDownType = CURSOR_SELECTION_NONE;

	double zoom = this->view->getXournal()->getZoom();
	//store translation matrix for pointer use
	double rx = (this->x + this->width/2) * zoom;
	double ry = (this->y + this->height/2) * zoom;

	cairo_matrix_init_identity(&this->cmatrix);
	cairo_matrix_translate(&this->cmatrix, rx, ry);
	cairo_matrix_rotate(&this->cmatrix, -this->rotation);
	cairo_matrix_translate(&this->cmatrix, -rx, -ry);
	
//	cairo_matrix_transform_point( &this->cmatrix, &x, &y);
	
	
	
	
}

/**
 * Handles mouse input for moving and resizing, coordinates are relative to "view"
 */
void EditSelection::mouseDown(CursorSelectionType type, double x, double y)
{
	double zoom = this->view->getXournal()->getZoom();


	if ( type != CURSOR_SELECTION_ROTATE && type != CURSOR_SELECTION_MOVE )	//NOT rotate nor move
	{
		cairo_matrix_transform_point( &this->cmatrix, &x, &y);
	}	
	x /= zoom;
	y /= zoom;
	
	this->mouseDownType = type;
	this->relMousePosX = x - this->x;
	this->relMousePosY = y - this->y;
}

/**
 * Handles mouse input for moving and resizing, coordinates are relative to "view"
 * 
 * For Move and Rotate the mouse position is not modified by the transformation matrix.
 * For Scale width and height it is... but the dx and dy moved to drag only one side has to be translated back out of the rotated coordinates.
 * 
 */
void EditSelection::mouseMove(double mouseX, double mouseY)
{
	double zoom = this->view->getXournal()->getZoom();
		
	if (this->mouseDownType == CURSOR_SELECTION_MOVE)
	{
		this->x = mouseX/zoom - this->relMousePosX;
		this->y = mouseY/zoom - this->relMousePosY;
	}
	else if (this->mouseDownType == CURSOR_SELECTION_ROTATE && supportRotation) // catch rotation here
	{
		double rdx = mouseX/zoom- this->x - this->width / 2;
		double rdy = mouseY/zoom - this->y - this->height / 2;

		double angle = atan2(rdy, rdx);
		this->rotation = angle;
	}
	else
	{
		//Translate mouse position into rotated coordinate system:
		double rx = mouseX;
		double ry = mouseY;
		cairo_matrix_transform_point( &this->cmatrix, &rx, &ry);
		rx /= zoom;
		ry /= zoom;
		
		double minSize = MINPIXSIZE/zoom;
		double dx = 0;
		double dy = 0;
		
		
		if (this->mouseDownType == CURSOR_SELECTION_TOP_LEFT)
		{
			dx = rx - this->x;
			dy = ry - this->y;
			double f;
			if (std::abs(dy) < std::abs(dx))
			{
				f = (this->height + dy) / this->height;
			}
			else
			{
				f = (this->width + dx) / this->width;
			}

			double oldW = this->width;
			double oldH = this->height;
			this->width /= f;
			this->height /= f;

			this->x = std::min( this->x + oldW - minSize, this->x + oldW - this->width);
			this->y = std::min( this->y + oldH - minSize, this->y + oldH - this->height);
		}
		else if (this->mouseDownType == CURSOR_SELECTION_TOP_RIGHT)
		{
			dx = rx - this->x - this->width;
			dy = ry - this->y;

			double f;
			if (std::abs(dy) < std::abs(dx))
			{
				f = this->height / (this->height + dy);
			}
			else
			{
				f = (this->width + dx) / this->width;
			}
			
			double oldH = this->height;
			this->width *= f;
			this->height *= f;

			this->y = std::min( this->y + oldH - minSize, this->y + oldH - this->height);
		}
		else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM_LEFT)
		{
			dx = rx - this->x;
			dy = ry - this->y - this->height;
			double f;
			if (std::abs(dy) < std::abs(dx))
			{
				f = (this->height + dy) / this->height;
			}
			else
			{
				f = this->width / (this->width + dx);
			}

			double oldW = this->width;
			this->width *= f;
			this->height *= f;

			this->x = std::min( this->x + oldW - minSize, this->x + oldW - this->width);
		}
		else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM_RIGHT)
		{
			dx = rx - this->x - this->width;
			dy = ry - this->y - this->height;
			double f;
			if (std::abs(dy) < std::abs(dx))
			{
				f = (this->height + dy) / this->height;
			}
			else
			{
				f = (this->width + dx) / this->width;
			}

			this->width *= f;
			this->height *= f;
		}
		else if (this->mouseDownType == CURSOR_SELECTION_TOP)
		{
			dy = ry - this->y;

			this->y = std::min( this->y+height-minSize, this->y +dy);
			this->height -= dy;

		}
		else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM)
		{
			dy = ry - this->y - this->height;
			this->height += dy;
		}
		else if (this->mouseDownType == CURSOR_SELECTION_LEFT)
		{
			dx = rx - this->x;
			this->x = std::min( this->x+width-minSize, this->x +dx);
			this->width -= dx;
		}
		else if (this->mouseDownType == CURSOR_SELECTION_RIGHT)	
		{
			dx = rx - this->x - this->width;
			this->width += dx;
			
		}


	//	this->width = std::max( this->width , minSize);
	//	this->height = std::max( this->height, minSize);
		
		if( this->width < minSize)
		{
				dx += minSize - this->width;
				this->width =  minSize;
		}
		if( this->height < minSize)
		{
				dy += minSize - this->height;
				this->height =  minSize;
		}
		
		// idea was to use dx and dy to accommodate scaling issues when rotated ( a bit of a hack) but we've got problems with undoScaling and everything would be better served if we re-wrote parts EditSelection and EditSelectionContents  so that scaling and rotation were all based off of center coordinates instead of topLeft.  Leaving for now to finish other checkins.
	
	}
	
	this->view->getXournal()->repaintSelection();

	XojPageView* v = getPageViewUnderCursor();

	if (v && v != this->view)
	{
		XournalView* xournal = this->view->getXournal();
		int pageNr = xournal->getControl()->getDocument()->indexOf(v->getPage());

		xournal->pageSelected(pageNr);

		translateToView(v);
	}
}

XojPageView* EditSelection::getPageViewUnderCursor()
{
	double zoom = view->getXournal()->getZoom();
	
	//get grabbing hand position
	double hx = this->view->getX() + (this->x + this->relMousePosX)*zoom;
	double hy = this->view->getY() + (this->y + this->relMousePosY)*zoom;
	
	
	Layout* layout = gtk_xournal_get_layout(this->view->getXournal()->getWidget());
	XojPageView* v = layout->getViewAt(hx,hy);

	return v;
}

/**
 * Translate all coordinates which are relative to the current view to the new view,
 * and set the attribute view to the new view
 */
void EditSelection::translateToView(XojPageView* v)
{
	double zoom = view->getXournal()->getZoom();

	int aX1 = getXOnViewAbsolute();
	int aY1 = getYOnViewAbsolute();

	this->x = (aX1 - v->getX()) / zoom;
	this->y = (aY1 - v->getY()) / zoom;

	this->view = v;

//	int aX2 = getXOnViewAbsolute();
//	int aY2 = getYOnViewAbsolute();
//
//	if (aX1 != aX2)
//	{
//		g_message("aX1 != aX2!! %i / %i", aX1, aX2);
//	}
//	if (aY1 != aY2)
//	{
//		g_message("aY1 != aY2!! %i / %i", aY1, aY2);
//	}
}

void EditSelection::copySelection()
{
	undo->addUndoAction(UndoActionPtr(contents->copySelection(this->view->getPage(), this->view, this->x, this->y)));
}

/**
 * If the selection should moved (or rescaled)
 */
bool EditSelection::isMoving()
{
	return this->mouseDownType != CURSOR_SELECTION_NONE;
}

/**
 * Move the selection
 */
void EditSelection::moveSelection(double dx, double dy)
{
	this->x -= dx;
	this->y -= dy;

	ensureWithinVisibleArea();

	this->view->getXournal()->repaintSelection();
}

/**
 * If the selection is outside the visible area correct the coordinates
 */
void EditSelection::ensureWithinVisibleArea()
{
	int viewx = this->view->getX();
	int viewy = this->view->getY();
	double zoom = this->view->getXournal()->getZoom();
	// need to modify this to take into account the position
	// of the object, plus typecast because XojPageView takes ints
	this->view->getXournal()->ensureRectIsVisible((int) (viewx + this->x * zoom), (int) (viewy + this->y * zoom),
												  (int) (this->width * zoom), (int) (this->height * zoom));
}

/**
 * Get the cursor type for the current position (if 0 then the default cursor should be used)
 */
CursorSelectionType EditSelection::getSelectionTypeForPos(double x, double y, double zoom)
{
	double x1 = getXOnView() * zoom;
	double x2 = x1 + (this->width * zoom);
	double y1 = getYOnView() * zoom;
	double y2 = y1 + (this->height * zoom);


	cairo_matrix_transform_point( &this->cmatrix, &x, &y);

	
	const int EDGE_PADDING = (this->btnWidth/2) + 2;
	const int BORDER_PADDING = (this->btnWidth/2);

	if (x1 - EDGE_PADDING <= x && x <= x1 + EDGE_PADDING && y1 - EDGE_PADDING <= y && y <= y1 + EDGE_PADDING)
	{
		return CURSOR_SELECTION_TOP_LEFT;
	}

	if (x2 - EDGE_PADDING <= x && x <= x2 + EDGE_PADDING && y1 - EDGE_PADDING <= y && y <= y1 + EDGE_PADDING)
	{
		return CURSOR_SELECTION_TOP_RIGHT;
	}

	if (x1 - EDGE_PADDING <= x && x <= x1 + EDGE_PADDING && y2 - EDGE_PADDING <= y && y <= y2 + EDGE_PADDING)
	{
		return CURSOR_SELECTION_BOTTOM_LEFT;
	}

	if (x2 - EDGE_PADDING <= x && x <= x2 + EDGE_PADDING && y2 - EDGE_PADDING <= y && y <= y2 + EDGE_PADDING)
	{
		return CURSOR_SELECTION_BOTTOM_RIGHT;
	}

	if ( x1 - (20+this->btnWidth) - BORDER_PADDING <=x &&  x1 - (20+this->btnWidth) + BORDER_PADDING >=x  &&  y1- BORDER_PADDING <= y	&&  y1 + BORDER_PADDING >= y)
	{
		return CURSOR_SELECTION_DELETE;
	}
	
	
	if (supportRotation && x2 - BORDER_PADDING + 8 + this->btnWidth <= x && x <= x2 + BORDER_PADDING + 8 + this->btnWidth  && (y2 + y1) / 2 - 4 - BORDER_PADDING <= y	&& (y2 + y1) / 2 + 4 + BORDER_PADDING>= y)
	{
		return CURSOR_SELECTION_ROTATE;
	}

	if (!this->aspectRatio)
	{
		if (x1 <= x && x2 >= x)
		{
			if (y1 - BORDER_PADDING <= y && y <= y1 + BORDER_PADDING)
			{
				return CURSOR_SELECTION_TOP;
			}

			if (y2 - BORDER_PADDING <= y && y <= y2 + BORDER_PADDING)
			{
				return CURSOR_SELECTION_BOTTOM;
			}
		}

		if (y1 <= y && y2 >= y)
		{
			if (x1 - BORDER_PADDING <= x && x <= x1 + BORDER_PADDING)
			{
				return CURSOR_SELECTION_LEFT;
			}

			if (x2 - BORDER_PADDING <= x && x <= x2 + BORDER_PADDING)
			{
				return CURSOR_SELECTION_RIGHT;
			}
		}
	}

	if (x1 <= x && x <= x2 && y1 <= y && y <= y2)
	{
		return CURSOR_SELECTION_MOVE;
	}

	return CURSOR_SELECTION_NONE;
}

void EditSelection::snapRotation()
{
	bool snapping = this->view->getXournal()->getControl()->getSettings()->isSnapRotation();
	if (!snapping)
	{
		return;
	}
	
	double epsilon = this->view->getXournal()->getControl()->getSettings()->getSnapRotationTolerance();
	
	const double ROTATION_LOCK[8] = {0, M_PI / 2.0, M_PI, M_PI / 4.0, 3.0 * M_PI / 4.0,
									- M_PI / 4.0, - 3.0 * M_PI / 4.0, - M_PI / 2.0};

	for (unsigned int i = 0; i < sizeof(ROTATION_LOCK) / sizeof(ROTATION_LOCK[0]); i++ )
	{
		if (std::abs(this->rotation - ROTATION_LOCK[i]) < epsilon)
		{
			this->rotation = ROTATION_LOCK[i];
		}
	}
}

/**
 * Paints the selection to cr, with the given zoom factor. The coordinates of cr
 * should be relative to the provided view by getView() (use translateEvent())
 */
void EditSelection::paint(cairo_t* cr, double zoom)
{
	double x = this->x;
	double y = this->y;
	

	if (std::abs(this->rotation) > __DBL_EPSILON__)
	{
		snapRotation();

		double rx = ((x + width / 2) - 0.7) * zoom;
		double ry = ((y + height / 2) - 0.7) * zoom;

		cairo_translate(cr, rx, ry);
		cairo_rotate(cr, this->rotation);

		// Draw the rotation point for debugging
		//cairo_set_source_rgb(cr, 0, 1, 0);
		//cairo_rectangle(cr, 0, 0, 10, 10);
		//cairo_stroke(cr);

		cairo_translate(cr, -rx, -ry);
	}
	this->contents->paint(cr, x, y, this->rotation, this->width, this->height, zoom);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	GtkColorWrapper selectionColor = view->getSelectionColor();

	// set the line always the same size on display
	cairo_set_line_width(cr, 1);

	const double dashes[] = {10.0, 10.0};
	cairo_set_dash(cr, dashes, sizeof(dashes) / sizeof(dashes[0]), 0);
	selectionColor.apply(cr);

	cairo_rectangle(cr, x * zoom, y * zoom, width * zoom, height * zoom);

	cairo_stroke_preserve(cr);
	selectionColor.applyWithAlpha(cr, 0.3);
	cairo_fill(cr);

	cairo_set_dash(cr, nullptr, 0, 0);

	if (!this->aspectRatio)
	{
		// top
		drawAnchorRect(cr, x + width / 2, y, zoom);
		// bottom
		drawAnchorRect(cr, x + width / 2, y + height, zoom);
		// left
		drawAnchorRect(cr, x, y + height / 2, zoom);
		// right
		drawAnchorRect(cr, x + width, y + height / 2, zoom);

		if (supportRotation)
		{
			// rotation handle
			drawAnchorRotation(cr, x + width + (8 + this->btnWidth)/zoom, y + height / 2, zoom);
		}
	}

	// top left
	drawAnchorRect(cr, x, y, zoom);
	// top right
	drawAnchorRect(cr, x + width, y, zoom);
	// bottom left
	drawAnchorRect(cr, x, y + height, zoom);
	// bottom right
	drawAnchorRect(cr, x + width, y + height, zoom);
	
	
	drawDeleteRect(cr, x - (20+this->btnWidth)/zoom, y  , zoom);
}

void EditSelection::drawAnchorRotation(cairo_t* cr, double x, double y, double zoom)
{
	GtkColorWrapper selectionColor = view->getSelectionColor();
	selectionColor.apply(cr);
	cairo_rectangle(cr, x * zoom - (this->btnWidth/2), y * zoom - (this->btnWidth/2), this->btnWidth, this->btnWidth);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgb(cr, 1, 0, 0);
	cairo_fill(cr);	
}

/**
 * draws an idicator where you can scale the selection
 */
void EditSelection::drawAnchorRect(cairo_t* cr, double x, double y, double zoom)
{
	GtkColorWrapper selectionColor = view->getSelectionColor();
	selectionColor.apply(cr);
	cairo_rectangle(cr, x * zoom - (this->btnWidth/2), y * zoom - (this->btnWidth/2), this->btnWidth, this->btnWidth);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);
}


/**
 * draws an idicator where you can delete the selection
 */
void EditSelection::drawDeleteRect(cairo_t* cr, double x, double y, double zoom)
{
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_rectangle(cr, x * zoom - (this->btnWidth/2), y * zoom - (this->btnWidth/2), this->btnWidth, this->btnWidth);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 1, 0, 0);
	cairo_move_to(cr, x * zoom - (this->btnWidth/2), y * zoom - (this->btnWidth/2));
	cairo_rel_move_to(cr, this->btnWidth,0);
	cairo_rel_line_to(cr, -this->btnWidth,this->btnWidth);
	cairo_rel_move_to(cr, this->btnWidth,0);
	cairo_rel_line_to(cr, -this->btnWidth,-this->btnWidth);
	cairo_stroke(cr);
}



XojPageView* EditSelection::getView()
{
	return this->view;
}

void EditSelection::serialize(ObjectOutputStream& out)
{
	out.writeObject("EditSelection");

	out.writeDouble(this->x);
	out.writeDouble(this->y);
	out.writeDouble(this->width);
	out.writeDouble(this->height);

	this->contents->serialize(out);
	out.endObject();

	out.writeInt(this->getElements()->size());
	for (Element* e : *this->getElements())
	{
		e->serialize(out);
	}
}

void EditSelection::readSerialized(ObjectInputStream& in)
{
	in.readObject("EditSelection");
	this->x = in.readDouble();
	this->y = in.readDouble();
	this->width = in.readDouble();
	this->height = in.readDouble();

	this->contents->readSerialized(in);

	in.endObject();
}
