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

#include "control/ToolHandler.h"

#include "AbstractToolItem.h"
#include "XournalType.h"

class ColorSelectImage;

class ColorToolItem: public AbstractToolItem {
public:
    ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, Color color,
                  bool selektor = false);
    virtual ~ColorToolItem();

public:
    virtual void actionSelected(ActionGroup group, ActionType action);
    void enableColor(Color color);
    virtual void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton);

    virtual string getToolDisplayName();
    virtual GtkWidget* getNewToolIcon();

    virtual string getId();

    Color getColor() const;

    /**
     * Enable / Disable the tool item
     */
    virtual void enable(bool enabled);

protected:
    virtual GtkToolItem* newItem();
    void updateName();
    bool isSelector();

    /**
     * Free the allocated icons
     */
    void freeIcons();

    /**
     * Show colochooser to select a custom color
     */
    void showColorchooser();

private:
    /**
     * Color
     */
    Color color;

    /**
     * Name of the Color
     */
    string name;

    /**
     * Icon to display
     */
    ColorSelectImage* icon = nullptr;

    GtkWindow* parent = nullptr;
    ToolHandler* toolHandler = nullptr;

    static bool inUpdate;
};
