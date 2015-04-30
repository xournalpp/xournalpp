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

class ToolPageLayer : public AbstractToolItem
{
public:
	ToolPageLayer(GladeGui* gui, ActionHandler* handler, string id, ActionType type);
	virtual ~ToolPageLayer();

public:
	static void cbSelectCallback(GtkComboBox* widget, ToolPageLayer* tpl);

	int getSelectedLayer();
	void setSelectedLayer(int selected);
	void setLayerCount(int layer, int selected);
	virtual string getToolDisplayName();

protected:
	virtual GtkToolItem* newItem();
	virtual GtkWidget* getNewToolIconImpl();

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* layerComboBox;
	GladeGui* gui;

	int layerCount;
	bool inCbUpdate;
};
