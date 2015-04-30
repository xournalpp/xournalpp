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

#pragma once

#include "model/LayerListener.h"
#include "model/TexImage.h"

#include <gtk/gtk.h>

#include <glib/gstdio.h>

class Control;

class LatexAction
{
public:
	LatexAction(string myTex, double tArea);

public:
	void runCommand();
	string getFileName();


private:
	//	Control * control;
	string theLatex;
	string texfile;
	string texfilefull;
	double myx, myy;
	double texArea;
};
