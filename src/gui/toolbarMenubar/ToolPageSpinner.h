/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "AbstractToolItem.h"
#include <XournalType.h>

class GladeGui;
class SpinPageAdapter;

class ToolPageSpinner : public AbstractToolItem
{
public:
	ToolPageSpinner(GladeGui* gui, ActionHandler* handler, string id, ActionType type);
	virtual ~ToolPageSpinner();

public:
	SpinPageAdapter* getPageSpinner();
	void setText(string text);
	virtual string getToolDisplayName();

protected:
	virtual GtkToolItem* newItem();
	virtual GtkWidget* getNewToolIconImpl();

private:
	XOJ_TYPE_ATTRIB;

	GladeGui* gui;

	SpinPageAdapter* pageSpinner;
	GtkWidget* lbPageNo;
};
