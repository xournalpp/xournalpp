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
#include "control/pagetype/PageTypeMenu.h"
#include "control/settings/PageTemplateSettings.h"

#include <XournalType.h>

class PageTypeHandler;
class PageTypeMenu;
class PageTypeInfo;

class PageTemplateDialog : public GladeGui, public PageTypeMenuChangeListener
{
public:
	PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types);
	virtual ~PageTemplateDialog();

public:
	virtual void show(GtkWindow* parent);

	/**
	 * The dialog was confirmed / saved
	 */
	bool isSaved();

	void pageSelected(PageTypeInfo* info);

private:
	void showPageSizeDialog();
	void updatePageSize();
	void saveToFile();
	void loadFromFile();
	void updateDataFromModel();
	void saveToModel();

private:
	XOJ_TYPE_ATTRIB;

	Settings* settings;

	PageTemplateSettings model;

	PageTypeMenu* pageMenu;

	/**
	 * The dialog was confirmed / saved
	 */
	bool saved;
};
