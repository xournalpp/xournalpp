#include "LatexController.h"

#include "Control.h"

#include "gui/XournalView.h"
#include "gui/dialog/LatexDialog.h"
#include "undo/InsertUndoAction.h"

#include <i18n.h>
#include <Util.h>
#include <Stacktrace.h>

#include <boost/filesystem.hpp>

#include <iostream>
using std::cout;
using std::endl;


LatexController::LatexController(Control* control)
 : control(control),
   texArea(0),
   // .png will be appended automatically => tex.png
   texImage(Util::getConfigFile("tex").string()),
   selectedTexImage(NULL),
   posx(0),
   posy(0),
   imgwidth(0),
   imgheight(0),
   doc(control->getDocument()),
   view(NULL),
   layer(NULL)
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

	path exePath = Stacktrace::getExePath();
	binTex = exePath.parent_path().string() + "/mathtex/mathtex-xournalpp.cgi";

	if (boost::filesystem::exists(binTex))
	{
		// Found binary in relative path
		return true;
	}

	gchar* mathtex = g_find_program_in_path("mathtex-xournalpp.cgi");
	if (!mathtex)
	{
		return false;
	}

	binTex = mathtex;
	g_free(mathtex);

	return true;
}


/**
 * Run LaTeX Command
 */
bool LatexController::runCommand()
{
	XOJ_CHECK_TYPE(LatexController);

	// can change font colour later with more features
	string fontcolour = "black";
	// dpi 300 is a good balance
	string texres;
	if (this->texArea < 1000)
	{
		texres = "300";
	}
	else if (this->texArea < 4000)
	{
		texres = "400";
	}
	else if (this->texArea < 8000)
	{
		texres = "500";
	}
	else if (this->texArea < 16000)
	{
		texres = "600";
	}
	else if (this->texArea < 32000)
	{
		texres = "800";
	}
	else
	{
		texres = "1000";
	}

	string command = FS(bl::format("{1} -m 0 \"\\png\\usepackage{{color}}\\color{{{2}}}\\dpi{{{3}}}\\normalsize {4}\" -o {5}")
						% binTex % "black" % texres
						% g_strescape(currentTex.c_str(), NULL) % texImage);

	gint rt = 0;
	void(*texhandler)(int) = signal(SIGCHLD, SIG_DFL);
	gboolean success = g_spawn_command_line_sync(command.c_str(), NULL, NULL, &rt, NULL);
	signal(SIGCHLD, texhandler);

	return success;
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

	if (selectedTexImage)
	{
		// this will get the position of the Latex properly
		EditSelection* theSelection = control->getWindow()->getXournal()->getSelection();
		posx = theSelection->getXOnView();
		posy = theSelection->getYOnView();

		imgwidth = selectedTexImage->getElementWidth();
		imgheight = selectedTexImage->getElementHeight();

		texArea = imgwidth * imgheight;
		initalTex = selectedTexImage->getText();
	}

	doc->unlock();

	// need to do this otherwise we can't remove the image for its replacement
	control->clearSelectionEndText();
}

void LatexController::showTexEditDialog()
{
	XOJ_CHECK_TYPE(LatexController);

	LatexDialog* dlg = new LatexDialog(control->getGladeSearchPath());
	dlg->setTex(initalTex);
	dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));
	currentTex = dlg->getTex();

	delete dlg;
}

void LatexController::deleteOldImage()
{
	XOJ_CHECK_TYPE(LatexController);

	if (selectedTexImage)
	{
		layer->removeElement(selectedTexImage, false);
		view->rerenderElement(selectedTexImage);
		delete selectedTexImage;
		selectedTexImage = NULL;
	}
}

void LatexController::insertTexImage()
{
	XOJ_CHECK_TYPE(LatexController);

	string imgPath = texImage + ".png";

	GFile* mygfile = g_file_new_for_path(imgPath.c_str());
	GError* err = NULL;
	GFileInputStream* in = g_file_read(mygfile, NULL, &err);
	g_object_unref(mygfile);

	if (err)
	{
		Util::showErrorToUser(control->getGtkWindow(), FS(_F("Could not retrieve LaTeX image file: {1}") % err->message));
		g_error_free(err);
		return;
	}

	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), NULL, &err);
	g_input_stream_close(G_INPUT_STREAM(in), NULL, NULL);


	doc->lock();

	deleteOldImage();

	TexImage* img = new TexImage();
	img->setX(posx);
	img->setY(posy);
	img->setImage(pixbuf);
	img->setText(currentTex);

	if (imgheight)
	{
		double ratio = (gdouble) gdk_pixbuf_get_width(pixbuf) / gdk_pixbuf_get_height(pixbuf);
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

	layer->addElement(img);
	view->rerenderElement(img);
	doc->unlock();

	control->getUndoRedoHandler()->addUndoAction(new InsertUndoAction(page, layer, img));
}

void LatexController::run()
{
	XOJ_CHECK_TYPE(LatexController);

	if (!findTexExecutable())
	{
		string msg = FS(_("Could not find Xournal++ LaTeX executable relative or in Path.\nSearched for: mathtex-xournalpp.cgi"));
		GtkWidget* dialog = gtk_message_dialog_new(control->getGtkWindow(),
												   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
												   msg.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), control->getGtkWindow());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}

	findSelectedTexElement();
	showTexEditDialog();

	if (currentTex.empty() || initalTex == currentTex)
	{
		// Nothing to insert / change
		return;
	}

	// now do all the LatexAction stuff
	if (!runCommand())
	{
		string msg = FS(_("Failed to generate LaTeX image!"));
		GtkWidget* dialog = gtk_message_dialog_new(control->getGtkWindow(),
												   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
												   msg.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), control->getGtkWindow());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}

	insertTexImage();
}

