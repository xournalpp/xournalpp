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

#ifndef __LATEXGLADE_H__
#define __LATEXGLADE_H__

#include "../../model/LayerListener.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "../../model/TexImage.h"
#include <XournalType.h>
#include "../../gui/GladeGui.h"

class LatexGlade : public GladeGui
{
public:
	LatexGlade(GladeSearchpath* gladeSearchPath);
	virtual ~LatexGlade();

public:
	virtual void show(GtkWindow* parent);
	void save();
	void load();
	void setTex(string texString);
	string getTex();


private:
	XOJ_TYPE_ATTRIB;
	GtkWidget* texBox;
	string theLatex;
};

#endif /* __LATEXGLADE_H__ */
