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

#include <memory>

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
	/**
	 * Open a LatexDialog, wait for the user to provide the LaTeX formula, and
	 * insert the rendered formula into the document if the supplied LaTeX is
	 * valid.
	 */
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
	 * Run the LaTeX command asynchronously. Note that this method can only be
	 * called when the preview is not updating.
	 *
	 * @return The PID of the spawned process, or nullptr if the .tex file could
	 * not be written or the command failed to start.
	 */
	std::unique_ptr<GPid> runCommandAsync();

	/**
	 * Asynchronously runs the LaTeX command and then updates the TeX image. If
	 * the preview is already being updated, then this method will be a no-op.
	 */
	void triggerImageUpdate();

	/**
	 * Show the LaTex Editor dialog
	 */
	void showTexEditDialog();

	/**
	 * Signal handler, updates the rendered image when the text in the editor
	 * changes.
	 */
	static void handleTexChanged(GtkTextBuffer* buffer, LatexController* self);

	/**
	 * Updates the display once the PDF file is generated.
	 *
	 * If the Latex text has changed since the last update, triggerPreviewUpdate
	 * will be called again.
	 */
	static void onPdfRenderComplete(GPid pid, gint returnCode, LatexController* self);

	void setUpdating(bool newValue);

	/*******/
	//Wrappers for signal handler who can't access non-static fields 
	//(see implementation for further explanation)
	void setImageInDialog(PopplerDocument* pdf);
	void setCurrentTex(string currentTex);
	GtkTextIter* getStartIterator(GtkTextBuffer* buffer);
	GtkTextIter* getEndIterator(GtkTextBuffer* buffer);
	/*******/

	/**
	 * Convert PDF Document to TexImage
	 */
	std::unique_ptr<TexImage> convertDocumentToImage(PopplerDocument* doc);

	/**
	 * Load PDF as TexImage
	 */
	std::unique_ptr<TexImage> loadRendered();

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
	 * The original TeX string when the dialog was opened, or the empty string
	 * if creating a new LaTeX element.
	 */
	string initialTex;

	/**
	 * The TeX string that the LaTeX element should display after editing
	 * finishes.
	 */
	string currentTex;

	/**
	 * The last TeX string shown in the preview.
	 */
	string lastPreviewedTex;

	/**
	 * Whether a preview is currently being generated.
	 */
	bool isUpdating = false;

	/**
	 * Whether the current TeX string is valid.
	 */
	bool isValidTex = false;

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
	 * The directory in which the LaTeX files will be generated. Note that this
	 * should be within a system temporary directory.
	 */
	Path texTmp;

	/**
	 * Previously existing TexImage
	 */
	TexImage* selectedTexImage = NULL;

	Text* selectedText = NULL;

	/**
	 * LaTex editor dialog
	 */
	std::unique_ptr<LatexDialog> dlg = nullptr;

	/**
	 * The controller owns the rendered preview in order to be able to delete it
	 * when a new render is created
	 */
	std::unique_ptr<TexImage> temporaryRender;
};
