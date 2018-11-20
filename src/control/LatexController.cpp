#include "LatexController.h"

#include "Control.h"

#include "gui/XournalView.h"
#include "gui/dialog/LatexDialog.h"

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
 * Delete selected tex image and keep Tex data
 */
void LatexController::deleteSelectedTexImag()
{

	int pageNr = control->getCurrentPageNo();
	if (pageNr == -1)
	{
		return;
	}
	XojPageView* view = control->getWindow()->getXournal()->getViewFor(pageNr);
	if (view == NULL)
	{
		return;
	}

	Document* doc = control->getDocument();

	doc->lock();
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

	doc->unlock();

	// need to do this otherwise we can't remove the image for its replacement
	control->clearSelectionEndText();

	return imgTex;
}

string LatexController::showTexEditDialog(string tex)
{
	LatexDialog* dlg = new LatexDialog(control->getGladeSearchPath());
	// determine if we should set a specific string
	dlg->setTex(texString);
	dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));
	string tex = dlg->getTex();

	delete dlg;

	return tex;
}

void LatexController::run()
{
//	texString = "";
//
//	string imgTex = deleteSelectedTexImag();
//
//	string texString = showTexEditDialog(imgTex);
//
//	if (texString.empty())
//	{
//		// Nothing to insert
//		// TODO Undo action!!!
//		return;
//	}
//
//	if (img)
//	{
//		layer->removeElement((Element*) img, false);
//		view->rerenderElement(img);
//		delete img;
//		img = NULL;
//	}
//
//	// now do all the LatexAction stuff
//	LatexAction texAction(tmp, imgheight * imgwidth);
//	texAction.runCommand();
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
//void LatexAction::runCommand()
//{
//	/*
//	 * at some point, I may need to sanitize theLatex
//	 */
//	cout << C_("LaTeX comand", "Command is being run.") << endl;
//	const gchar* mtex = "mathtex-xournalpp.cgi";
//	gchar* mathtex = g_find_program_in_path(mtex);
//	if (!mathtex)
//	{
//		cerr << _("Error: problem finding mathtex. Doing nothing…") << endl;
//		return;
//	}
//	cout << _F("Found mathtex in your path! TeX area is {1}") % this->texArea << endl;
//	g_free(mathtex);
//	//can change font colour later with more features
//	string fontcolour = "black";
//	//dpi 300 is a good balance
//	string texres;
//	if (this->texArea < 1000)
//	{
//		texres = "300";
//	}
//	else if (this->texArea < 4000)
//	{
//		texres = "400";
//	}
//	else if (this->texArea < 8000)
//	{
//		texres = "500";
//	}
//	else if (this->texArea < 16000)
//	{
//		texres = "600";
//	}
//	else if (this->texArea < 32000)
//	{
//		texres = "800";
//	}
//	else
//	{
//		texres = "1000";
//	}
//	string command = FS(bl::format("{1} -m 0 \"\\png\\usepackage{{color}}\\color{{{2}}}\\dpi{{{3}}}\\normalsize {4}\" -o {5}")
//						% mtex % (fontcolour.length() ? fontcolour : "black") % texres
//						% g_strescape(this->theLatex.c_str(), NULL) % this->texfile);
//
//	gint rt = 0;
//	void(*texhandler)(int) = signal(SIGCHLD, SIG_DFL);
//	gboolean success = g_spawn_command_line_sync(command.c_str(), NULL, NULL, &rt, NULL);
//	signal(SIGCHLD, texhandler);
//	if (!success)
//	{
//		cout << _("LaTeX command execution failed.") << endl;
//		return;
//	}
//	cout << _F("LaTeX command: \"{1}\" executed successfully. Result saved to file {2}.") % this->theLatex % this->texfilefull << endl;
//
//}
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



