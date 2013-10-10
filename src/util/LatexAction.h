/*
 * Xournal++
 *
 * Latex implementation
 *
 * @author W Brenna
 * http://wbrenna.ca
 *
 * @license GPL
 */

#ifndef __LATEXACTION_H__
#define __LATEXACTION_H__

#include "../model/LayerListener.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "../model/TexImage.h"


class Control;

class LatexAction {
public:
	LatexAction(gchar * myTex);
	virtual ~LatexAction();

public:
	void runCommand();
	gchar * getFileName();


private:
//	Control * control;
	gchar * theLatex;
	gchar * texfile;
	gchar * texfilefull;
	double myx, myy;
};

#endif /* __LATEXACTION_H__ */
