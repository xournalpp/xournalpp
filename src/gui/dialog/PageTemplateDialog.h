/*
 * Xournal++
 *
 * Dialog to configure page template for new pages
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/settings/Settings.h"
#include "gui/GladeGui.h"
#include "model/BackgroundType.h"
#include "control/settings/PageTemplateSettings.h"

#include <XournalType.h>

#include <vector>

using std::vector;

typedef struct {
	string name;
	BackgroundType type;
} PageFormat;

class PageTemplateDialog : public GladeGui
{
public:
	PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings);
	virtual ~PageTemplateDialog();

public:
	virtual void show(GtkWindow* parent);

private:
	void showPageSizeDialog();
	void updatePageSize();

private:
	XOJ_TYPE_ATTRIB;

	Settings* settings;

	PageTemplateSettings model;

	vector<PageFormat> formatList;
};
