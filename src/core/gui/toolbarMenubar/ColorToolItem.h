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

#include <memory>
#include <string>
#include <vector>

#include "util/NamedColor.h"
#include "AbstractToolItem.h"

class ColorSelectImage;
class ToolHandler;

class ColorToolItem: public AbstractToolItem {
public:
    ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, NamedColor namedColor,
                  bool selektor = false);
    ColorToolItem(ColorToolItem const&) = delete;
    ColorToolItem(ColorToolItem &&) noexcept = delete;
    auto operator=(ColorToolItem const&) -> ColorToolItem& = delete;
    auto operator=(ColorToolItem &&) noexcept -> ColorToolItem&  = delete;
    ~ColorToolItem() override;


public:
    void actionSelected(ActionGroup group, ActionType action) override;
    void enableColor(Color color);
    void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton) override;

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
    std::unique_ptr<ColorSelectImage> icon{};

    GtkWindow* parent = nullptr;
    ToolHandler* toolHandler = nullptr;

    static bool inUpdate;
};
