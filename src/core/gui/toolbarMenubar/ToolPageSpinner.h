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

#include "AbstractToolItem.h"  // for AbstractToolItem

class SpinPageAdapter;

class ToolPageSpinner: public AbstractToolItem {
public:
    ToolPageSpinner(std::string id, IconNameHelper iconNameHelper);
    ~ToolPageSpinner() override;

public:
    SpinPageAdapter* getPageSpinner() const;
    void setPageInfo(size_t pagecount, size_t pdfpage);
    std::string getToolDisplayName() const override;
    GtkWidget* createItem(bool horizontal) override;

protected:
    GtkWidget* getNewToolIcon() const override;

private:
    void updateLabels();

private:
    std::unique_ptr<SpinPageAdapter> pageSpinner;
    GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;

    xoj::util::WidgetSPtr lbPageNo;

    /** The current page of the document. */
    size_t pageCount = 0;

    /** The current page in the background PDF, or 0 if there is no PDF. */
    size_t pdfPage = 0;

    IconNameHelper iconNameHelper;
};
