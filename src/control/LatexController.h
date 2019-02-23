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

#include <Path.h>
#include <XournalType.h>
#include "gui/dialog/LatexDialog.h"

#include <poppler.h>

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
	void setImageInDialog(PopplerDocument* pdf);
	void deletePreviousRender();
	void setCurrentTex(string currentTex);
	GtkTextIter* getStartIterator(GtkTextBuffer* buffer);
	GtkTextIter* getEndIterator(GtkTextBuffer* buffer);
	/*******/

	/**
	 * Convert PDF Document to TexImage
	 */
	TexImage* convertDocumentToImage(PopplerDocument* doc);

	/**
	 * Load PDF as TexImage
	 */
	TexImage* loadRendered();

	/**
	 * Actual image creation
	 */
	void insertTexImage();

private:
	XOJ_TYPE_ATTRIB;

	Control* control = NULL;

	/**
	 * Tex binary full path
	 */
	Path binTex;

	/**
	 * Orignal TeX, if editing
	 */
	string initalTex;

	/**
	 * Updated TeX string
	 */
	string currentTex;

	/**
	 * X-Position
	 */
	int posx = 0;

	/**
	 * Y-Position
	 */
	int posy = 0;

	/**
	 * Image width
	 */
	int imgwidth = 0;

	/**
	 * Image height
	 */
	int imgheight = 0;

	/**
	 * Document
	 */
	Document* doc = NULL;

	/**
	 * Page View
	 */
	XojPageView* view = NULL;

	/**
	 * Selected Page
	 */
	PageRef page;

	/**
	 * Selected layer
	 */
	Layer* layer = NULL;

	/**
	 * Text tmp directory in configuration folder
	 */
	string texTmp;

	/**
	 * Previously existin TexImage
	 */
	TexImage* selectedTexImage = NULL;

	Text* selectedText = NULL;

	/**
	 * LaTex editor dialog
	 */
	LatexDialog* dlg = NULL;

	/**
	 * The controller holds the 'on-the-go' render in order
	 * to be able to delete it when a new render is created
	 */
	TexImage* temporaryRender = NULL;


	/**
	 * TextBuffer iterators
	 * I don't understand why, but declaring these as pointers
	 * makes the TextView widget crash
	 */
	GtkTextIter start;
	GtkTextIter end;
};
