/*
 * Xournal++
 *
 * The about dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"

class AboutDialog : public GladeGui
{
public:
	AboutDialog(GladeSearchpath* gladeSearchPath);
	virtual ~AboutDialog();

public:
	virtual void show(GtkWindow* parent);

private:
	};
