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


class ColorSelectImage;

class ColorToolItem: public AbstractToolItem {
public:
    ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, NamedColor namedColor,
                  bool selektor = false);
    virtual ~ColorToolItem();

public:
    virtual void actionSelected(ActionGroup group, ActionType action);
    void enableColor(Color color);
    virtual void activated(GdkEvent* event, GtkButton* menuitem, GtkButton* toolbutton);

    virtual std::string getToolDisplayName() const;
    virtual GtkWidget* getNewToolIcon() const;
    virtual GdkPixbuf* getNewToolPixbuf() const;

    std::string getId() const final;

    Color getColor() const;

    /**
     * Enable / Disable the tool item
     */
    virtual void enable(bool enabled);

protected:
    GtkWidget* newItem() override;
    bool isSelector() const;

    /**
     * Free the allocated icons
     */
    void freeIcons();

    /**
     * Show colochooser to select a custom color
     */
    void showColorchooser();

private:
    NamedColor namedColor;

    /**
     * Icon to display
     */
    ColorSelectImage* icon = nullptr;

    GtkWindow* parent = nullptr;
    ToolHandler* toolHandler = nullptr;

    static bool inUpdate;
};
