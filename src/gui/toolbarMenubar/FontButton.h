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

#include <string>
#include <vector>

#include "gui/GladeGui.h"
#include "model/Font.h"

#include "AbstractToolItem.h"


class FontButton: public AbstractToolItem {
public:
    FontButton(ActionHandler* handler, GladeGui* gui, std::string id, ActionType type, std::string description,
               GtkWidget* menuitem = nullptr);
    virtual ~FontButton();

public:
    virtual void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton);
    void setFont(XojFont& font);
    XojFont getFont();
    virtual std::string getToolDisplayName();
    void showFontDialog();

protected:
    virtual GtkToolItem* createItem(bool horizontal);
    virtual GtkToolItem* createTmpItem(bool horizontal);
    virtual GtkToolItem* newItem();

    static GtkWidget* newFontButton();
    static void setFontFontButton(GtkWidget* fontButton, XojFont& font);

    virtual GtkWidget* getNewToolIcon();

private:
    GtkWidget* fontButton = nullptr;
    GladeGui* gui = nullptr;
    std::string description;

    XojFont font;
};
