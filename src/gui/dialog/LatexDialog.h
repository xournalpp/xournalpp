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

#include "gui/GladeGui.h"

#include <XournalType.h>

class LatexDialog : public GladeGui
{
public:
	LatexDialog(GladeSearchpath* gladeSearchPath);
	virtual ~LatexDialog();

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
