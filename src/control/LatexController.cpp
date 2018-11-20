#include "LatexController.h"

#include "Control.h"

#include "gui/XournalView.h"
#include "gui/dialog/LatexDialog.h"

#include <i18n.h>

#include <iostream>
using std::cout;
using std::endl;


LatexController::LatexController(Control* control)
 : control(control)
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

	binTex = "mathtex/mathtex-xournalpp.cgi";

	if (g_file_test(binTex.c_str(), G_FILE_TEST_EXISTS))
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
void LatexController::runCommand()
{
	XOJ_CHECK_TYPE(LatexController);

	/*
	//can change font colour later with more features
	string fontcolour = "black";
	//dpi 300 is a good balance
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
						% mtex % (fontcolour.length() ? fontcolour : "black") % texres
						% g_strescape(this->theLatex.c_str(), NULL) % this->texfile);

	gint rt = 0;
	void(*texhandler)(int) = signal(SIGCHLD, SIG_DFL);
	gboolean success = g_spawn_command_line_sync(command.c_str(), NULL, NULL, &rt, NULL);
	signal(SIGCHLD, texhandler);
	if (!success)
	{
		cout << _("LaTeX command execution failed.") << endl;
		return;
	}
	cout << _F("LaTeX command: \"{1}\" executed successfully. Result saved to file {2}.") % this->theLatex % this->texfilefull << endl;

	*/

}

/**
 * Find a selected tex element, and load it
 */
void LatexController::findSelectedTexElement()
{
	XOJ_CHECK_TYPE(LatexController);

	Document* doc = control->getDocument();
	doc->lock();
	int pageNr = control->getCurrentPageNo();
	if (pageNr == -1)
	{
		doc->unlock();
		return;
	}
	XojPageView* view = control->getWindow()->getXournal()->getViewFor(pageNr);
	if (view == NULL)
	{
		doc->unlock();
		return;
	}
/*
	// we get the selection
	PageRef page = doc->getPage(pageNr);
	Layer* layer = page->getSelectedLayer();

	TexImage* img = view->getSelectedTex();

	double imgx = 10;
	double imgy = 10;
	double imgheight = 0;
	double imgwidth = 0;
	string imgTex;
	if (img)
	{
		// this will get the position of the Latex properly
		EditSelection* theSelection = control->getWindow()->getXournal()->getSelection();
		imgx = theSelection->getXOnView();
		imgy = theSelection->getYOnView();

		imgheight = img->getElementHeight();
		imgwidth = img->getElementWidth();
		imgTex = img->getText();
	}
*/
	doc->unlock();

	// need to do this otherwise we can't remove the image for its replacement
//	control->clearSelectionEndText();
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

//	if (img)
//	{
//		layer->removeElement((Element*) img, false);
//		view->rerenderElement(img);
//		delete img;
//		img = NULL;
//	}

	// now do all the LatexAction stuff
	runCommand();
//
//	doc->lock();
//
//	GFile* mygfile = g_file_new_for_path(texAction.getFileName().c_str());
//	cout << "About to insert image...";
//	GError* err = NULL;
//	GFileInputStream* in = g_file_read(mygfile, NULL, &err);
//	g_object_unref(mygfile);
//	if (err)
//	{
//		doc->unlock();
//
//		// TODO cerr << _F("Could not retrieve LaTeX image file: {1}") % err->message << endl;
//
//		g_error_free(err);
//		return;
//	}
//
//	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), NULL, &err);
//	g_input_stream_close(G_INPUT_STREAM(in), NULL, NULL);
//
//	img = new TexImage();
//	img->setX(imgx);
//	img->setY(imgy);
//	img->setImage(pixbuf);
//	img->setText(tmp);
//
//	if (imgheight)
//	{
//		double ratio = (gdouble) gdk_pixbuf_get_width(pixbuf) / gdk_pixbuf_get_height(pixbuf);
//		if (ratio == 0)
//		{
//			if (imgwidth == 0)
//			{
//				img->setWidth(10);
//			}
//			else
//			{
//				img->setWidth(imgwidth);
//			}
//		}
//		else
//		{
//			img->setWidth(imgheight * ratio);
//		}
//		img->setHeight(imgheight);
//	}
//	else
//	{
//		img->setWidth(gdk_pixbuf_get_width(pixbuf));
//		img->setHeight(gdk_pixbuf_get_height(pixbuf));
//	}
//
//	layer->addElement(img);
//	view->rerenderElement(img);
//
//
//	cout << "LaTeX Image inserted!" << endl;
//
//	doc->unlock();
//
//	control->getUndoRedoHandler()->addUndoAction(new InsertUndoAction(page, layer, img));
}











//#include "LatexAction.h"
//
//#include "control/Control.h"
//#include "control/tools/ImageHandler.h"
//#include "gui/PageView.h"
//#include "gui/XournalView.h"
//#include "model/Stroke.h"
//#include "model/Layer.h"
//#include "model/TexImage.h"
////TODO some time - clean up these includes
//#include "serializing/HexObjectEncoding.h"
//#include "serializing/ObjectOutputStream.h"
//
//#include <i18n.h>
//
//#include <glib.h>
//
//#include <iostream>
//using namespace std;
//
//LatexAction::LatexAction(string myTex, double tArea)
//{
//	//this->control = control;
//
//	this->theLatex = myTex;
//
//	this->texfile = Util::getConfigFile("tex").string();
//	this->texfilefull = Util::getConfigFile("tex.png").string();
//
//	cout << this->texfile << endl;
//
//	//set up the default positions.
//	this->myx = 0;
//	this->myy = 0;
//
//	this->texArea = tArea;
//}
//
//
//string LatexAction::getFileName()
//{
//	//gets the filename for our image
//	return this->texfilefull;
//}




/*
 * Xournal++
 *
 * Latex implementation
 *
 * @author W Brenna
 * http://wbrenna.ca
 *
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

//#pragma once
//
//#include "model/LayerListener.h"
//#include "model/TexImage.h"
//
//#include <glib/gstdio.h>
//#include <gtk/gtk.h>
//
//class Control;
//
//class LatexAction
//{
//public:
//	LatexAction(string myTex, double tArea);
//
//public:
//	void runCommand();
//	string getFileName();
//
//private:
//	//	Control * control;
//	string theLatex;
//	string texfile;
//	string texfilefull;
//	double myx, myy;
//	double texArea;
//};



