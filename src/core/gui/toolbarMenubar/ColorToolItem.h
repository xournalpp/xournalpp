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

#include <gtk/gtk.h>  // for GtkWidget

#include "util/Color.h"                 // for Color
#include "util/NamedColor.h"            // for NamedColor

#include "AbstractToolItem.h"

class ActionDatabase;
class OpacityToolbox;

class ColorToolItem: public AbstractToolItem {
public:
    ColorToolItem(NamedColor namedColor, OpacityToolbox* opacityToolbox);
    ColorToolItem(ColorToolItem const&) = delete;
    ColorToolItem(ColorToolItem&&) noexcept = delete;
    auto operator=(ColorToolItem const&) -> ColorToolItem& = delete;
    auto operator=(ColorToolItem&&) noexcept -> ColorToolItem& = delete;
    ~ColorToolItem() override;


public:
    std::string getToolDisplayName() const override;
    GtkWidget* getNewToolIcon() const override;

    Color getColor() const;

    GtkWidget* createItem(bool horizontal) override;

private:
    NamedColor namedColor;
    xoj::util::GVariantSPtr target;  ///< Contains the color in ARGB as a uint32_t

    OpacityToolbox* opacityToolbox;
    xoj::util::GObjectSPtr<GtkEventController> enterLeaveController;
    xoj::util::GObjectSPtr<GtkGesture> singleClickGesture;

    static void handleRelease(GtkGesture* gesture, int n_press, double x, double y, ColorToolItem* self);
    static bool handleEnter(GtkEventController* eventController, gdouble x, gdouble y, ColorToolItem* self);
    static bool handleLeave(GtkEventController* eventController, ColorToolItem* self);
};
