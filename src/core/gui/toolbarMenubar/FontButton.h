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
    ~FontButton() override;

public:
    void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton) override;
    void setFont(XojFont& font);
    XojFont getFont() const;
    std::string getToolDisplayName() const override;
    void showFontDialog();

protected:
    GtkToolItem* createItem(bool horizontal) override;
    GtkToolItem* createTmpItem(bool horizontal) override;
    GtkToolItem* newItem() override;

    static GtkWidget* newFontButton();
    static void setFontFontButton(GtkWidget* fontButton, XojFont& font);

    GtkWidget* getNewToolIcon() const override;
    GdkPixbuf* getNewToolPixbuf() const override;

private:
    GtkWidget* fontButton = nullptr;
    GladeGui* gui = nullptr;
    std::string description;

    XojFont font;
};
