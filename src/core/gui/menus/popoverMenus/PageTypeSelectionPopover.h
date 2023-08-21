/*
 * Xournal++
 *
 * Handles page selection menu
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget

#include "gui/menus/PageTypeSelectionMenuBase.h"
#include "util/raii/GObjectSPtr.h"

class PageBackgroundChangeController;
class PageTypeHandler;
class PageTypeInfo;
class Settings;

class PageTypeSelectionPopover: public PageTypeSelectionMenuBase {
public:
    PageTypeSelectionPopover(PageTypeHandler* typesHandler, PageBackgroundChangeController* controller,
                             const Settings* settings, GtkApplicationWindow* win);
    ~PageTypeSelectionPopover() = default;

public:
    inline GtkWidget* getPopover() { return popover.get(); }

private:
    xoj::util::WidgetSPtr createPopover();
    void entrySelected(const PageTypeInfo* info) override;

private:
    PageBackgroundChangeController* controller;

    xoj::util::WidgetSPtr applyToCurrentPageButton;
    xoj::util::WidgetSPtr popover;
};
