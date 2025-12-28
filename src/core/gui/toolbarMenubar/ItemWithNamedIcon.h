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

#include <string>  // for string

#include <gtk/gtk.h>  // for GtkWidget

#include "AbstractToolItem.h"

class ActionDatabase;

class ItemWithNamedIcon: public AbstractToolItem {
public:
    ItemWithNamedIcon(std::string id, Category cat);
    ~ItemWithNamedIcon() override = default;

public:
    GtkWidget* getNewToolIcon() const override;

    /// Returns a GdkPaintable representing the tool, tailored to be displayed on the provided surface
    xoj::util::GObjectSPtr<GdkPaintable> createPaintable(GdkSurface* target) const override;

    virtual const char* getIconName() const = 0;
};
