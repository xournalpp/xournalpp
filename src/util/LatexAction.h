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
 * @license GNU GPLv3
 */

#ifndef __LATEXACTION_H__
#define __LATEXACTION_H__

#include "../model/LayerListener.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "../model/TexImage.h"


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

#endif /* __LATEXACTION_H__ */
