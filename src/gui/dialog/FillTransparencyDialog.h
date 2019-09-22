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

class FillTransparencyDialog : public GladeGui
{
public:
	FillTransparencyDialog(GladeSearchpath* gladeSearchPath, int alpha);
	virtual ~FillTransparencyDialog();

public:
	virtual void show(GtkWindow* parent);

	int getResultAlpha();

private:
	void setPreviewImage(int alpha);

private:
	int resultAlpha = -1;
};
