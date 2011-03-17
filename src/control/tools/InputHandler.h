/*
 * Xournal++
 *
 * Handles input and optimizes the stroke
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __INPUTHANDLER_H__
#define __INPUTHANDLER_H__

#include <gtk/gtk.h>
#include "../../model/Stroke.h"
#include "../../model/Page.h"
#include "../../util/MemoryCheck.h"
#include "../shaperecognizer/ShapeRecognizer.h"

class DocumentView;
class XournalView;
class PageView;

class InputHandler: public MemoryCheckObject {
public:
	InputHandler(XournalView * xournal, PageView * redrawable);
	virtual ~InputHandler();

public:
	void addPointToTmpStroke(GdkEventMotion * event);
	void draw(cairo_t * cr, double zoom);
	void onButtonReleaseEvent(GdkEventButton * event, XojPage * page);
	bool onMotionNotifyEvent(GdkEventMotion * event);
	void startStroke(GdkEventButton * event, StrokeTool tool, double x, double y);

	Stroke * getTmpStroke();

	void resetShapeRecognizer();
private:
	bool getPressureMultiplier(GdkEvent * event, double & presure);
	void drawTmpStroke();

private:
	XournalView * xournal;

	/**
	 * If you are drawing on the document
	 */
	Stroke * tmpStroke;

	/**
	 * What has already be drawed, only draw the new part
	 */
	int tmpStrokeDrawElem;

	/**
	 * The current input device for stroken, do not react on other devices (linke mices)
	 */
	GdkDevice * currentInputDevice;

	/**
	 * The view widget for drawing
	 */
	GtkWidget * widget;

	/**
	 * The View to draw the stroke
	 */
	DocumentView * view;

	/**
	 * The view which should be refreshed
	 */
	PageView * redrawable;

	/**
	 * Xournal shape recognizer, one instance per page
	 */
	ShapeRecognizer * reco;
};

#endif /* __INPUTHANDLER_H__ */
