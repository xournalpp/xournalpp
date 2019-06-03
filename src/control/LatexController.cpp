#include "LatexController.h"

#include "Control.h"

#include "gui/XournalView.h"
#include "gui/dialog/LatexDialog.h"
#include "undo/InsertUndoAction.h"

#include <i18n.h>
#include <Util.h>
#include <Stacktrace.h>
#include <XojMsgBox.h>
#include <StringUtils.h>

#include "pixbuf-utils.h"

/**
 * First half of the LaTeX template used to generate preview PDFs. User-supplied
 * formulas will be inserted between the two halves.
 *
 * This template is necessarily complicated because we need to cause an error if
 * the rendered formula is blank. Otherwise, a completely blank, sizeless PDF
 * will be generated, which Poppler will be unable to load.
 */
const char* LATEX_TEMPLATE_1 =
	R"(\documentclass[crop, border=5pt]{standalone})" "\n"
	R"(\usepackage{amsmath})" "\n"
	R"(\usepackage{amssymb})" "\n"
	R"(\usepackage{ifthen})" "\n"
	R"(\newlength{\pheight})" "\n"
	R"(\def\preview{\(\displaystyle)" "\n";

const char* LATEX_TEMPLATE_2 =
	"\n\\)}\n"
	R"(\begin{document})" "\n"
	R"(\settoheight{\pheight}{\preview} %)" "\n"
	R"(\ifthenelse{\pheight=0})" "\n"
	R"({\GenericError{}{xournalpp: blank formula}{}{}})" "\n"
	R"(\preview)" "\n"
	R"(\end{document})" "\n";

LatexController::LatexController(Control* control)
	: control(control),
	  doc(control->getDocument()),
	  texTmp(Util::getTmpDirSubfolder("tex"))
{
	XOJ_INIT_TYPE(LatexController);
	Util::ensureFolderExists(this->texTmp);
}

LatexController::~LatexController()
{
	XOJ_CHECK_TYPE(LatexController);

	this->control = NULL;

	XOJ_RELEASE_TYPE(LatexController);
}

/**
 * Find the tex executable, return false if not found
 */
bool LatexController::findTexExecutable()
{
	XOJ_CHECK_TYPE(LatexController);

	gchar* pdflatex = g_find_program_in_path("pdflatex");
	if (!pdflatex)
	{
		return false;
	}

	binTex = pdflatex;
	g_free(pdflatex);

	return true;
}

std::unique_ptr<GPid> LatexController::runCommandAsync()
{
	XOJ_CHECK_TYPE(LatexController);
	g_assert(!this->isUpdating);

	string texContents = LATEX_TEMPLATE_1;
	texContents += this->currentTex;
	texContents += LATEX_TEMPLATE_2;

	Path texFile = texTmp / "tex.tex";

	GError* err = NULL;
	if (!g_file_set_contents(texFile.c_str(), texContents.c_str(), texContents.length(), &err))
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), FS(_F("Could not save .tex file: {1}") % err->message));
		g_error_free(err);
		return nullptr;
	}

	char* texFileEscaped = g_strescape(texFile.c_str(), NULL);
	char* cmd = g_strdup(binTex.c_str());

	static char* texFlag = g_strdup("-interaction=nonstopmode");
	char* argv[] = { cmd, texFlag, texFileEscaped, NULL };

	std::unique_ptr<GPid> pdflatex_pid(new GPid);
	GSpawnFlags flags = GSpawnFlags(G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_DO_NOT_REAP_CHILD);
	this->setUpdating(true);
	this->lastPreviewedTex = this->currentTex;
	bool success = g_spawn_async(texTmp.c_str(), argv, nullptr, flags, nullptr, nullptr, pdflatex_pid.get(), &err);
	if (!success)
	{
		string message = FS(_F("Could not start pdflatex: {1} (exit code: {2})") % err->message % err->code);
		g_warning("%s", message.c_str());
		XojMsgBox::showErrorToUser(control->getGtkWindow(), message);

		g_error_free(err);
		this->setUpdating(false);
		pdflatex_pid.reset();
	}

	g_free(texFileEscaped);
	g_free(cmd);

	return pdflatex_pid;
}

/**
 * Find a selected tex element, and load it
 */
void LatexController::findSelectedTexElement()
{
	XOJ_CHECK_TYPE(LatexController);

	this->doc->lock();
	int pageNr = this->control->getCurrentPageNo();
	if (pageNr == -1)
	{
		this->doc->unlock();
		return;
	}
	this->view = this->control->getWindow()->getXournal()->getViewFor(pageNr);
	if (view == NULL)
	{
		this->doc->unlock();
		return;
	}

	// we get the selection
	this->page = this->doc->getPage(pageNr);
	this->layer = page->getSelectedLayer();

	this->selectedTexImage = view->getSelectedTex();
	this->selectedText = view->getSelectedText();

	if (this->selectedTexImage || this->selectedText)
	{
		// this will get the position of the Latex properly
		EditSelection* theSelection = control->getWindow()->getXournal()->getSelection();
		this->posx = theSelection->getXOnView();
		this->posy = theSelection->getYOnView();

		if (this->selectedTexImage != nullptr)
		{
			this->initialTex = this->selectedTexImage->getText();
			this->imgwidth = this->selectedTexImage->getElementWidth();
			this->imgheight = this->selectedTexImage->getElementHeight();
		}
		else
		{
			this->initialTex += "\\text{";
			this->initialTex += this->selectedText->getText();
			this->initialTex += "}";
			this->imgwidth = this->selectedText->getElementWidth();
			this->imgheight = this->selectedText->getElementHeight();
		}
	}
	this->doc->unlock();
	this->currentTex = this->initialTex;
	if (this->initialTex.empty())
	{
		this->currentTex = "x^2";
	}

	// need to do this otherwise we can't remove the image for its replacement
	this->control->clearSelectionEndText();
}

void LatexController::showTexEditDialog()
{
	XOJ_CHECK_TYPE(LatexController);

	this->dlg.reset(new LatexDialog(control->getGladeSearchPath()));

	// preselect default text so user can overwrite it easily
	this->dlg->setTex(currentTex, this->initialTex.empty());

	g_signal_connect(dlg->getTextBuffer(), "changed", G_CALLBACK(handleTexChanged), this);

	if (this->temporaryRender != nullptr)
	{
		this->dlg->setTempRender(this->temporaryRender->getPdf());
	}

	this->dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));

	this->currentTex = this->dlg->getTex();
	// If the user cancelled, there is no change in the latex string.
	this->currentTex = this->currentTex == "" ? initialTex : this->currentTex;

	this->dlg.reset();
}

void LatexController::triggerImageUpdate()
{
	if (this->isUpdating)
	{
		return;
	}

	std::unique_ptr<GPid> pid = this->runCommandAsync();
	if (pid != nullptr)
	{
		g_assert(this->isUpdating);
		g_child_watch_add(*pid, reinterpret_cast<GChildWatchFunc>(onPdfRenderComplete), this);
	}
}

/**
 * Text-changed handler: when the Buffer in the dialog changes, this handler
 * updates currentTex, removes the previous existing render and creates a new
 * one. We need to do it through 'self' because signal handlers cannot directly
 * access non-static methods and non-static fields such as 'dlg' so we need to
 * wrap all the dlg method inside small methods in 'self'. To improve
 * performance, we render the text asynchronously.
 */
void LatexController::handleTexChanged(GtkTextBuffer* buffer, LatexController* self)
{
	XOJ_CHECK_TYPE_OBJ(self, LatexController);

	GtkTextIter start;
	GtkTextIter end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	self->setCurrentTex(gtk_text_buffer_get_text(buffer, &start, &end, true));

	self->triggerImageUpdate();
}

void LatexController::onPdfRenderComplete(GPid pid, gint returnCode, LatexController* self)
{
	XOJ_CHECK_TYPE_OBJ(self, LatexController);
	g_assert(self->isUpdating);
	GError* err = nullptr;
	g_spawn_check_exit_status(returnCode, &err);
	g_spawn_close_pid(pid);
	bool shouldUpdate = self->lastPreviewedTex != self->currentTex;
	if (err != nullptr)
	{
		self->isValidTex = false;
		if (!g_error_matches(err, G_SPAWN_EXIT_ERROR, 1))
		{
			// The error was not caused by invalid LaTeX.
			string message = FS(_F("pdflatex encountered an error: {1} (exit code: {2})") % err->message % err->code);
			g_warning("%s", message.c_str());
			XojMsgBox::showErrorToUser(self->control->getGtkWindow(), message);
		}
		Path pdfPath = self->texTmp / "tex.pdf";
		if (pdfPath.exists())
		{
			// Delete the pdf to prevent more errors
			pdfPath.deleteFile();
		}
		g_error_free(err);
	}
	else
	{
		self->isValidTex = true;
		self->temporaryRender = self->loadRendered();
		if (self->temporaryRender != nullptr)
		{
			self->setImageInDialog(self->temporaryRender->getPdf());
		}

	}
	self->setUpdating(false);
	if (shouldUpdate)
	{
		self->triggerImageUpdate();
	}
}

void LatexController::setUpdating(bool newValue)
{
	XOJ_CHECK_TYPE(LatexController);
	GtkWidget* okButton = this->dlg->get("texokbutton");
	bool buttonEnabled = true;
	if ((!this->isUpdating && newValue) || (this->isUpdating && !newValue))
	{
		// Disable LatexDialog OK button while updating. This is a workaround
		// for the fact that 1) the LatexController only lives while the dialog
		// is open; 2) the preview is generated asynchronously; and 3) the `run`
		// method that inserts the TexImage object is called synchronously after
		// the dialog is closed with the OK button.
		buttonEnabled = !newValue;
	}


	// Invalid LaTeX will generate an invalid PDF, so disable the OK button if
	// needed.
	buttonEnabled = buttonEnabled ? this->isValidTex : buttonEnabled;

	gtk_widget_set_sensitive(okButton, buttonEnabled);

	GtkLabel* errorLabel = GTK_LABEL(this->dlg->get("texErrorLabel"));
	gtk_label_set_text(errorLabel, this->isValidTex ? "" : "The formula is empty when rendered or invalid.");

	this->isUpdating = newValue;
}

void LatexController::setImageInDialog(PopplerDocument* pdf)
{
	XOJ_CHECK_TYPE(LatexController);
	this->dlg->setTempRender(pdf);
}

void LatexController::setCurrentTex(string currentTex)
{
	XOJ_CHECK_TYPE(LatexController);
	this->currentTex = currentTex;
}

void LatexController::deleteOldImage()
{
	XOJ_CHECK_TYPE(LatexController);

	if (this->selectedTexImage != nullptr)
	{
		g_assert(this->selectedText == nullptr);
		EditSelection selection(control->getUndoRedoHandler(), selectedTexImage, view, page);
		this->view->getXournal()->deleteSelection(&selection);
		this->selectedTexImage = nullptr;
	}
	else if (this->selectedText)
	{
		g_assert(this->selectedTexImage == nullptr);
		EditSelection selection(control->getUndoRedoHandler(), selectedText, view, page);
		view->getXournal()->deleteSelection(&selection);
		this->selectedText = nullptr;
	}
}

std::unique_ptr<TexImage> LatexController::convertDocumentToImage(PopplerDocument* doc)
{
	XOJ_CHECK_TYPE(LatexController);

	if (poppler_document_get_n_pages(doc) < 1)
	{
		return NULL;
	}

	PopplerPage* page = poppler_document_get_page(doc, 0);


	double pageWidth = 0;
	double pageHeight = 0;
	poppler_page_get_size(page, &pageWidth, &pageHeight);

	std::unique_ptr<TexImage> img(new TexImage());
	img->setX(posx);
	img->setY(posy);
	img->setText(currentTex);

	if (imgheight)
	{
		double ratio = pageWidth / pageHeight;
		if (ratio == 0)
		{
			if (imgwidth == 0)
			{
				img->setWidth(10);
			}
			else
			{
				img->setWidth(imgwidth);
			}
		}
		else
		{
			img->setWidth(imgheight * ratio);
		}
		img->setHeight(imgheight);
	}
	else
	{
		img->setWidth(pageWidth);
		img->setHeight(pageHeight);
	}

	return img;
}

/**
 * Load PDF as TexImage
 */
std::unique_ptr<TexImage> LatexController::loadRendered()
{
	XOJ_CHECK_TYPE(LatexController);

	if (!this->isValidTex)
	{
		return nullptr;
	}

	Path pdfPath = texTmp / "tex.pdf";
	GError* err = NULL;

	gchar* fileContents = NULL;
	gsize fileLength = 0;
	if (!g_file_get_contents(pdfPath.c_str(), &fileContents, &fileLength, &err))
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(),
				FS(_F("Could not load LaTeX PDF file, File Error: {1}") % err->message));
		g_error_free(err);
		return NULL;
	}

	PopplerDocument* pdf = poppler_document_new_from_data(fileContents, fileLength, NULL, &err);
	if (err != NULL)
	{
		string message = FS(_F("Could not load LaTeX PDF file: {1}") % err->message);
		g_message("%s", message.c_str());
		XojMsgBox::showErrorToUser(control->getGtkWindow(), message);
		g_error_free(err);
		return NULL;
	}

	if (pdf == NULL)
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), FS(_F("Could not load LaTeX PDF file")));
		return NULL;
	}

	std::unique_ptr<TexImage> img = convertDocumentToImage(pdf);
	g_object_unref(pdf);

	// Do not assign the PDF, theoretical it should work, but it gets a Poppler PDF error
	// img->setPdf(pdf);
	img->setBinaryData(string(fileContents, fileLength));

	g_free(fileContents);

	return img;
}

void LatexController::insertTexImage()
{
	XOJ_CHECK_TYPE(LatexController);

	TexImage* img = this->loadRendered().release();
	g_assert(img != nullptr);

	this->deleteOldImage();

	doc->lock();
	layer->addElement(img);
	view->rerenderElement(img);
	doc->unlock();

	control->getUndoRedoHandler()->addUndoAction(new InsertUndoAction(page, layer, img));

	// Select element
	EditSelection* selection = new EditSelection(control->getUndoRedoHandler(), img, view, page);
	view->getXournal()->setSelection(selection);
}

void LatexController::run()
{
	XOJ_CHECK_TYPE(LatexController);

	if (!this->findTexExecutable())
	{
		string msg = _("Could not find pdflatex in Path.\nPlease install pdflatex first and make sure it's in the PATH.");
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		return;
	}

	this->findSelectedTexElement();
	this->showTexEditDialog();

	if (this->initialTex != this->currentTex)
	{
		g_assert(this->isValidTex);
		this->insertTexImage();
	}
}
