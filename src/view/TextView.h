/*
 * Xournal++
 *
 * Displays a text Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <StringUtils.h>
#include <gtk/gtk.h>

class Text;

class TextView
{
private:
	TextView();
	virtual ~TextView();

public:
	/**
	 * Calculates the size of a Text model
	 */
	static void calcSize(Text* t, double& width, double& height);

	/**
	 * Draws a Text modle to a cairo surface
	 */
	static void drawText(cairo_t* cr, Text* t);

	/**
	 * Searches text within a Text model, returns XojPopplerRectangle, have to been freed
	 */
	static GList* findText(Text* t, string& text);

	/**
	 * Initialize a Pango layout
	 */
	static PangoLayout* initPango(cairo_t* cr, Text* t);

	/**
	 * Sets the font name from Text model
	 */
	static void updatePangoFont(PangoLayout* layout, Text* t);

	//Sets up DPI
	static int getDPI();
};
