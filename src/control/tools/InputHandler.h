/*
 * Xournal++
 *
 * Handles input and optimizes the stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>
#include "model/Stroke.h"
#include "model/PageRef.h"
#include "control/shaperecognizer/ShapeRecognizer.h"
#include "gui/inputdevices/PositionInputData.h"

#include <XournalType.h>

class DocumentView;
class XournalView;
class XojPageView;

/**
 * @brief A base class to handle pointer input
 * 
 * The InputHandler receives various events from a XojPageView
 * and updates the XojPageView to display strokes being
 * drawn
 */
class InputHandler
{
public:
	InputHandler(XournalView* xournal, XojPageView* redrawable, PageRef page);
	virtual ~InputHandler();

public:

	 /**
	 * This method is called from the XojPageView to draw
	 * overlays displaying the drawing process.
	 * It is called from XojPageView::paintPage(cairo_t* cr, GdkRectangle* rect)
	 * 
	 * @remark The coordinate system is in XojPageView coordinates, scale
	 *         it by the current zoom to change to Page coordinates
	 */
	virtual void draw(cairo_t* cr) = 0;

	/**
	 * This method is called from the XojPageView as soon
	 * as the pointer is moved while this InputHandler
	 * is active. It is used to update internal data
	 * structures and queue repaints of the XojPageView
	 * if necessary
	 */
	virtual bool onMotionNotifyEvent(const PositionInputData& pos) = 0;

	/**
	 * This method is called from the XojPageView when a keypress is detected.
	 * It is used to update internal data structures and queue 
	 * repaints of the XojPageView if necessary.
	 */
	virtual bool onKeyEvent(GdkEventKey* event) = 0;
	
 	/**
	 * The current input device for stroken, do not react on other devices (linke mices)
	 * This method is called from the XojPageView as soon
	 * as the pointer is released.
	 */
	virtual void onButtonReleaseEvent(const PositionInputData& pos) = 0;

 	/**
	 * This method is called from the XojPageView as soon
	 * as the pointer is pressed.
	 */
	virtual void onButtonPressEvent(const PositionInputData& pos) = 0;

	/**
	 * @return Current editing stroke
	 */
	Stroke* getStroke();

	/**
	 * Reset the shape recognizer, only implemented by drawing instances,
	 * but needs to be in the base interface.
	 */
	virtual void resetShapeRecognizer();
	
	/**
	 * userTapped - experimental feature to take action on filtered draw. See cbDoActionOnStrokeFilter
	 */
	bool userTapped = false;	

protected:

	bool validMotion(Point p, Point q);

	void createStroke(Point p);

protected:
	XournalView* xournal;
	XojPageView* redrawable;
	PageRef page;
	Stroke* stroke;

private:
	};
