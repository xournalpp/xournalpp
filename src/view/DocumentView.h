/*
 * Xournal++
 *
 * Paints a page to a cairo context
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <gtk/gtk.h>
#include "../model/Element.h"
#include "../model/Stroke.h"
#include "../model/Text.h"
#include "../model/PageRef.h"
#include "../model/Image.h"
#include "../model/TexImage.h"
#include <XournalType.h>

#include "ElementContainer.h"

class EditSelection;

class DocumentView
{
public:
	DocumentView();
	virtual ~DocumentView();

public:
	void drawPage(PageRef page, cairo_t* cr, bool preview);
	void drawStroke(cairo_t* cr, Stroke* s, int startPoint = 0,
					double scaleFactor = 1);
	void drawEraseableStroke(cairo_t* cr, Stroke* s);
	static void applyColor(cairo_t* cr, int c, int alpha = 255);
	static void applyColor(cairo_t* cr, Element* e, int alpha = 255);

	void limitArea(double x, double y, double width, double heigth);

	void drawSelection(cairo_t* cr, ElementContainer* container);

private:
	void drawText(cairo_t* cr, Text* t);
	void drawImage(cairo_t* cr, Image* i);
	void drawTexImage(cairo_t* cr, TexImage* i);

	void drawElement(cairo_t* cr, Element* e);
	void drawLayer(cairo_t* cr, Layer* l);

	void paintBackgroundImage();
	void paintBackgroundColor();
	void paintBackgroundGraph();
	void paintBackgroundRuled();
	void paintBackgroundLined();

private:
	XOJ_TYPE_ATTRIB;

	cairo_t* cr;
	PageRef page;
	double width;
	double height;
	bool dontRenderEditingStroke;

	double lX;
	double lY;
	double lWidth;
	double lHeight;
};
