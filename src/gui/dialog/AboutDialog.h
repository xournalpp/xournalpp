/*
 * Xournal++
 *
 * The about dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "../GladeGui.h"
#include <XournalType.h>

class AboutDialog : public GladeGui
{
public:
	AboutDialog(GladeSearchpath* gladeSearchPath);
	virtual ~AboutDialog();

public:
	virtual void show(GtkWindow* parent);

private:
	XOJ_TYPE_ATTRIB;
};
