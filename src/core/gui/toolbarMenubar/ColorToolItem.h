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

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gdk/gdk.h>                // for GdkEvent
#include <gtk/gtk.h>                // for GtkWindow, GtkMenuItem, GtkToolB...

#include "enums/ActionGroup.enum.h"  // for ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType
#include "util/Color.h"              // for Color
#include "util/NamedColor.h"         // for NamedColor

#include "AbstractToolItem.h"  // for AbstractToolItem

class ColorSelectImage;
class ToolHandler;
class ActionHandler;

class ColorToolItem: public AbstractToolItem {
public:
    ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, NamedColor namedColor,
                  bool selector = false);
    ColorToolItem(ColorToolItem const&) = delete;
    ColorToolItem(ColorToolItem&&) noexcept = delete;
    auto operator=(ColorToolItem const&) -> ColorToolItem& = delete;
    auto operator=(ColorToolItem&&) noexcept -> ColorToolItem& = delete;
    ~ColorToolItem() override;


public:
    void actionSelected(ActionGroup group, ActionType action) override;
    void enableColor(Color color);
    void activated(GtkMenuItem* menuitem, GtkToolButton* toolbutton) override;

    std::string getToolDisplayName() const override;
    GtkWidget* getNewToolIcon() const override;
    GdkPixbuf* getNewToolPixbuf() const override;

    std::string getId() const final;

    Color getColor() const;

    /**
     * Enable / Disable the tool item
     */
    void enable(bool enabled) override;

protected:
    GtkToolItem* newItem() override;
    bool isSelector() const;

    /**
     * Show colochooser to select a custom color
     */
    void showColorchooser();

private:
    NamedColor namedColor;

    /**
     * Icon to display
     */
    std::unique_ptr<ColorSelectImage> icon;

    GtkWindow* parent = nullptr;
    ToolHandler* toolHandler = nullptr;

    static bool inUpdate;
};
