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

namespace xoj::popup {
class PageTemplateDialog;
}
class PageTypeHandler;
class PageTypeInfo;
class Settings;

class PageTypeSelectionPopoverGridOnly final: public PageTypeSelectionMenuBase {
public:
    PageTypeSelectionPopoverGridOnly(PageTypeHandler* typesHandler, const Settings* settings,
                                     xoj::popup::PageTemplateDialog* parent);
    ~PageTypeSelectionPopoverGridOnly() = default;

public:
    inline GtkWidget* getPopover() { return popover.get(); }

private:
    xoj::util::WidgetSPtr createPopover();
    void entrySelected(const PageTypeInfo* info) override;

private:
    xoj::popup::PageTemplateDialog* parent;
    xoj::util::WidgetSPtr popover;
};
