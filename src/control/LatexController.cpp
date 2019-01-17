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

const char* LATEX_TEMPLATE_1 =
"\\documentclass[border=5pt]{standalone}\n"
"\\usepackage{amsmath}\n"
"\\begin{document}\n"
"\\(\\displaystyle\n";

const char* LATEX_TEMPLATE_2 =
"\n\\)\n"
"\\end{document}\n";

LatexController::LatexController(Control* control)
	: control(control),
	  posx(0),
	  posy(0),
	  imgwidth(0),
	  imgheight(0),
	  doc(control->getDocument()),
	  view(NULL),
	  layer(NULL),
	  texTmp(Util::getConfigSubfolder("tex").str()),
	  selectedTexImage(NULL),
	  selectedText(NULL),
	  dlg(NULL),
	  temporaryRender(NULL)
{
	XOJ_INIT_TYPE(LatexController);
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

/**
 * Run LaTeX Command
 */
bool LatexController::runCommand()
{
	XOJ_CHECK_TYPE(LatexController);

	string texContents = LATEX_TEMPLATE_1;
	texContents += currentTex;
	texContents += LATEX_TEMPLATE_2;

	string texFile = texTmp + "/tex.tmp";

	GError* err = NULL;
	if (!g_file_set_contents(texFile.c_str(), texContents.c_str(), texContents.length(), &err))
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), FS(_F("Could not save .tex file: {1}") % err->message));
		g_error_free(err);
		return false;
	}

	char* texFileEscaped = g_strescape(texFile.c_str(), NULL);
	char* cmd = g_strdup(binTex.str().c_str());

	char* argv[] = { cmd, texFileEscaped, NULL };

	gint returnCode = 0;
	if (!g_spawn_sync(texTmp.c_str(), argv, NULL, GSpawnFlags(G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL),
			NULL, NULL, NULL, NULL, &returnCode, &err))
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), FS(_F("Could not convert tex to PDF: {1} (exit code: {2})") % err->message % returnCode));
		g_error_free(err);
		g_free(texFileEscaped);
		g_free(cmd);
		return false;
	}

	g_free(texFileEscaped);
	g_free(cmd);

	return true;
}

/**
 * Find a selected tex element, and load it
 */
void LatexController::findSelectedTexElement()
{
	XOJ_CHECK_TYPE(LatexController);

	doc->lock();
	int pageNr = control->getCurrentPageNo();
	if (pageNr == -1)
	{
		doc->unlock();
		return;
	}
	view = control->getWindow()->getXournal()->getViewFor(pageNr);
	if (view == NULL)
	{
		doc->unlock();
		return;
	}

	// we get the selection
	page = doc->getPage(pageNr);
	layer = page->getSelectedLayer();

	selectedTexImage = view->getSelectedTex();
	selectedText = view->getSelectedText();

	if (selectedTexImage || selectedText)
	{
		// this will get the position of the Latex properly
		EditSelection *theSelection = control->getWindow()->getXournal()->getSelection();
		posx = theSelection->getXOnView();
		posy = theSelection->getYOnView();

		if (selectedTexImage)
		{
			initalTex = selectedTexImage->getText();
			imgwidth = selectedTexImage->getElementWidth();
			imgheight = selectedTexImage->getElementHeight();
		}
		else
		{
			initalTex += "\\text{";
			initalTex += selectedText->getText();
			initalTex += "}";
			imgwidth = selectedText->getElementWidth();
			imgheight = selectedText->getElementHeight();
		}
	}
	if (initalTex.empty())
	{
		initalTex = "x^2";
	}
	currentTex = initalTex;
	doc->unlock();

	// need to do this otherwise we can't remove the image for its replacement
	control->clearSelectionEndText();
}

void LatexController::showTexEditDialog()
{
	XOJ_CHECK_TYPE(LatexController);

	dlg = new LatexDialog(control->getGladeSearchPath());

	// For 'real time' LaTex rendering in the dialog
	dlg->setTex(initalTex);
	g_signal_connect(dlg->getTextBuffer(), "changed", G_CALLBACK(handleTexChanged), this);
	
	
	// The controller owns the tempRender because, on signal changed, he has to handle the old/new renders
	if (temporaryRender != NULL)
	{
		dlg->setTempRender(temporaryRender->getImage(), initalTex.size());
	}

	dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));

	deletePreviousRender();
	currentTex = dlg->getTex();
	currentTex += " ";

	delete dlg;
}

/**
 * Text-changed handler: when the Buffer in the dialog changes,
 * this handler updates currentTex, removes the previous existing render and creates
 * a new one. We need to do it through 'self' because signal handlers
 * cannot directly access non-static methods and non-static fields such as
 * 'dlg' so we need to wrap all the dlg method inside small methods in 'self'
 */
void LatexController::handleTexChanged(GtkTextBuffer* buffer, LatexController* self)
{
	XOJ_CHECK_TYPE_OBJ(self, LatexController);
	
	//Right now, this is the only way I know to extract text from TextBuffer
	self->setCurrentTex(gtk_text_buffer_get_text(buffer, self->getStartIterator(buffer), self->getEndIterator(buffer), TRUE));
	self->deletePreviousRender();
	self->runCommand();
	self->insertTexImage(true);

	if (self->getTemporaryRender() != NULL)
	{
		self->setImageInDialog(self->getTemporaryRender()->getImage());
	}
}

TexImage* LatexController::getTemporaryRender()
{
	XOJ_CHECK_TYPE(LatexController);
	return this->temporaryRender;
}

void LatexController::setImageInDialog(cairo_surface_t* image)
{
	XOJ_CHECK_TYPE(LatexController);
	dlg->setTempRender(image, currentTex.size());
}

void LatexController::deletePreviousRender()
{
	XOJ_CHECK_TYPE(LatexController);
	delete temporaryRender;
	temporaryRender = NULL;
}

void LatexController::setCurrentTex(string currentTex)
{
	XOJ_CHECK_TYPE(LatexController);
	this->currentTex = currentTex;
}

GtkTextIter* LatexController::getStartIterator(GtkTextBuffer* buffer)
{
	XOJ_CHECK_TYPE(LatexController);
	gtk_text_buffer_get_start_iter(buffer, &this->start);
	return &this->start;
}

GtkTextIter* LatexController::getEndIterator(GtkTextBuffer* buffer)
{
	XOJ_CHECK_TYPE(LatexController);
	gtk_text_buffer_get_end_iter(buffer, &this->end);
	return &this->end;
}

void LatexController::deleteOldImage()
{
	XOJ_CHECK_TYPE(LatexController);

	if (selectedTexImage)
	{
		EditSelection* selection = new EditSelection(control->getUndoRedoHandler(), selectedTexImage, view, page);
		view->getXournal()->deleteSelection(selection);
		delete selection;
		selectedTexImage = NULL;
	}
	else if (selectedText)
	{
		EditSelection* selection = new EditSelection(control->getUndoRedoHandler(), selectedText, view, page);
		view->getXournal()->deleteSelection(selection);
		delete selection;
		selectedText = NULL;
	}
}

void LatexController::insertTexImage(bool forTemporaryRender)
{
	XOJ_CHECK_TYPE(LatexController);

	string imgPath = texTmp + ".png";
	GFile* mygfile = g_file_new_for_path(imgPath.c_str());
	GError* err = NULL;
	GFileInputStream* in = g_file_read(mygfile, NULL, &err);
	g_object_unref(mygfile);

	if (err)
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), FS(_F("Could not retrieve LaTeX image file: {1}") % err->message));
		g_error_free(err);
		return;
	}

	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), NULL, &err);
	g_input_stream_close(G_INPUT_STREAM(in), NULL, NULL);

	deleteOldImage();

	TexImage* img = new TexImage();
	img->setX(posx);
	img->setY(posy);
	img->setImage(pixbuf);
	img->setText(currentTex);

	if (imgheight)
	{
		double ratio = (gdouble)gdk_pixbuf_get_width(pixbuf) / gdk_pixbuf_get_height(pixbuf);
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
		img->setWidth(gdk_pixbuf_get_width(pixbuf));
		img->setHeight(gdk_pixbuf_get_height(pixbuf));
	}

	//Calling this function for 'on-the-go' LaTex render too, so
	//if this is that case, return to the caller at this point
	if (forTemporaryRender)
	{
		this->temporaryRender = img;
		return;
	}

	doc->lock();
	layer->addElement(img);
	view->rerenderElement(img);
	doc->unlock();

	control->getUndoRedoHandler()->addUndoAction(new InsertUndoAction(page, layer, img));

	// Select element
	EditSelection* selection = new EditSelection(control->getUndoRedoHandler(), img, view, page);
	view->getXournal()->setSelection(selection);

	return;
}

void LatexController::run()
{
	XOJ_CHECK_TYPE(LatexController);

	if (!findTexExecutable())
	{
		string msg = _("Could not find pdflatex in Path.");
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		return;
	}

	findSelectedTexElement();
	showTexEditDialog();

	if (StringUtils::trim(currentTex).empty() || initalTex == currentTex)
	{
		// Nothing to insert / change
		return;
	}

	// now do all the LatexAction stuff
	if (!runCommand())
	{
		string msg = _("Failed to generate LaTeX image!");
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		return;
	}

	//False, because it's not for temporary render but this time
	//we add it to the document
	insertTexImage(false);
}
