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
#include "gui/GladeGui.h"
#include "model/Font.h"

#include <XournalType.h>

class FontButton : public AbstractToolItem
{
public:
	FontButton(ActionHandler* handler, GladeGui* gui, string id, ActionType type,
			   string description, GtkWidget* menuitem = nullptr);
	virtual ~FontButton();

public:
	virtual void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton);
	void setFont(XojFont& font);
	XojFont getFont();
	virtual string getToolDisplayName();
	void showFontDialog();

protected:
	virtual GtkToolItem* createItem(bool horizontal);
	virtual GtkToolItem* createTmpItem(bool horizontal);
	virtual GtkToolItem* newItem();

	GtkWidget* newFontButton();
	static void setFontFontButton(GtkWidget* fontButton, XojFont& font);

	virtual GtkWidget* getNewToolIcon();

private:
	GtkWidget* fontButton = nullptr;
	GladeGui* gui = nullptr;
	string description;

	XojFont font;
};
