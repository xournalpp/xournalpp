/*
 * Xournal++
 *
 * Controller for Latex stuff
 *
 * @author W Brenna
 * http://wbrenna.ca
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"
#include "model/Text.h"

#include <XournalType.h>
#include "gui/dialog/LatexDialog.h"

class Control;
class TexImage;
class Text;
class Document;
class XojPageView;
class Layer;

class LatexController
{
public:
	LatexController(Control* control);
	virtual ~LatexController();

public:
	void run();

private:
	/**
	 * Find the tex executable, return false if not found
	 */
	bool findTexExecutable();

	/**
	 * Find a selected tex element, and load it
	 */
	void findSelectedTexElement();

	/**
	 * If a previous image/text is selected, delete it 
	 */
	void deleteOldImage();

	/**
	 * Run LaTeX Command
	 */
	bool runCommand();

	/**
	 * Show the LaTex Editor dialog
	 */
	void showTexEditDialog();

	/**
	 * Signal handler, updates the rendered image when the text in the editor changes
	 */
	static void handleTexChanged(GtkTextBuffer* buffer, LatexController* self);

	/*******/
	//Wrappers for signal handler who can't access non-static fields 
	//(see implementation for further explanation)
	TexImage* getTemporaryRender();
	void setImageInDialog(cairo_surface_t* image);
	void deletePreviousRender();
	void setCurrentTex(string currentTex);
	GtkTextIter* getStartIterator(GtkTextBuffer* buffer);
	GtkTextIter* getEndIterator(GtkTextBuffer* buffer);
	/*******/

	/**
	 * Actual image creation, if 'forTemporaryRender' is true, it does not
	 * add the image to the doc because it means that it has been called
	 * during the render in the Editor dialog
	 */
	void insertTexImage(bool forTemporaryRender);

private:
	XOJ_TYPE_ATTRIB;

	Control* control;

	/**
	 * Tex binary full path
	 */
	string binTex;

	/**
	 * Orignal TeX, if editing
	 */
	string initalTex;

	/**
	 * Updated TeX string
	 */
	string currentTex;

	/**
	 * Image area
	 */
	int texArea;

	/**
	 * X-Position
	 */
	int posx;

	/**
	 * Y-Position
	 */
	int posy;

	/**
	 * Image width
	 */
	int imgwidth;

	/**
	 * Image height
	 */
	int imgheight;

	/**
	 * Document
	 */
	Document* doc;

	/**
	 * Page View
	 */
	XojPageView* view;

	/**
	 * Selected Page
	 */
	PageRef page;

	/**
	 * Selected layer
	 */
	Layer* layer;

	/**
	 * Image is temporary stored in configuration folder
	 */
	string texImage;

	/**
	 * Previously existin TexImage
	 */
	TexImage* selectedTexImage;

	Text* selectedText;

	/**
	 * LaTex editor dialog
	 */
	LatexDialog* dlg;

	/**
	 * The controller holds the 'on-the-go' render in order
	 * to be able to delete it when a new render is created
	 */
	TexImage* temporaryRender;


	/**
	 * TextBuffer iterators
	 * I don't understand why, but declaring these as pointers
	 * makes the TextView widget crash
	 */
	GtkTextIter start;
	GtkTextIter end;
};
