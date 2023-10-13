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

#include "gui/PopoverFactory.h"
#include "gui/menus/PageTypeSelectionMenuBase.h"
#include "util/raii/GObjectSPtr.h"

class PageBackgroundChangeController;
class PageTypeHandler;
class PageTypeInfo;
class Settings;

class PageTypeSelectionPopover final: public PageTypeSelectionMenuBase, public PopoverFactory {
public:
    PageTypeSelectionPopover(PageTypeHandler* typesHandler, PageBackgroundChangeController* controller,
                             const Settings* settings, GtkApplicationWindow* win);
    ~PageTypeSelectionPopover() override = default;

public:
    GtkWidget* createPopover() const override;

private:
    void entrySelected(const PageTypeInfo* info) override;

private:
    PageBackgroundChangeController* controller;
};
