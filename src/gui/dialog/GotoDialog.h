/*
 * Xournal++
 *
 * Goto-Page dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"

class GotoDialog : public GladeGui
{
public:
	GotoDialog(GladeSearchpath* gladeSearchPath, int maxPage);
	virtual ~GotoDialog();

public:
	virtual void show(GtkWindow* parent);

	// returns the selected page or -1 if closed
	int getSelectedPage();

private:
	int selectedPage = -1;
};
