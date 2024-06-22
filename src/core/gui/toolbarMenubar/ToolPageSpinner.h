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

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkWidget, GtkToolItem, GTK_ORIEN...

#include "gui/IconNameHelper.h"     // for IconNameHelper
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

#include "ItemWithNamedIcon.h"  // for ItemWithNamedIcon

class SpinPageListener;

class ToolPageSpinner: public ItemWithNamedIcon {
public:
    ToolPageSpinner(std::string id, IconNameHelper iconNameHelper, SpinPageListener* listener);
    ~ToolPageSpinner() override;

public:
    /// Propagates the info to all instances of the Page Spinner
    void setPageInfo(size_t currentPage, size_t pageCount, size_t pdfPage);
    std::string getToolDisplayName() const override;
    Widgetry createItem(ToolbarSide side) override;

    inline SpinPageListener* getListener() const { return listener; }

protected:
    const char* getIconName() const override;

private:
    SpinPageListener* listener;
    std::string iconName;

    class Instance;
    std::vector<Instance*> instances;
};
