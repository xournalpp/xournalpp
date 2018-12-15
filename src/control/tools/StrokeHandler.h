/*
 * Xournal++
 *
 * Handles input of strokes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "InputHandler.h"

#include "view/DocumentView.h"

class ShapeRecognizer;

/**
 * @brief The stroke handler draws a stroke on a XojPageView
 * 
 * The stroke is drawn using a cairo_surface_t* as a mask:
 * As the pointer moves on the canvas single segments are
 * drawn opaquely on the initially transparent masking
 * surface. The surface is used to mask the stroke
 * when drawing it to the XojPageView
 */
class StrokeHandler : public InputHandler
{
public:
	StrokeHandler(XournalView* xournal, XojPageView* redrawable, PageRef page);
	virtual ~StrokeHandler();

	void draw(cairo_t* cr);

	bool onMotionNotifyEvent(const PositionInputData& pos);
	void onButtonReleaseEvent(const PositionInputData& pos);
	void onButtonPressEvent(const PositionInputData& pos);

	/**
	 * Reset the shape recognizer, only implemented by drawing instances,
	 * but needs to be in the base interface.
	 */
	virtual void resetShapeRecognizer();

private:
	XOJ_TYPE_ATTRIB;

	void destroySurface();

	/**
	 * The masking surface
	 */
	cairo_surface_t* surfMask;

	/**
	 * And the corresponding cairo_t*
	 */
	cairo_t* crMask;

	DocumentView view;
	Rectangle visRect;

	ShapeRecognizer* reco;
};

