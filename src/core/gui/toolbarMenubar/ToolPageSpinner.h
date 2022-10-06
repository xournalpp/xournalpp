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
#include <string>   // for string

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkWidget, GtkToolItem, GTK_ORIEN...

#include "enums/ActionType.enum.h"  // for ActionType
#include "gui/IconNameHelper.h"     // for IconNameHelper

#include "AbstractToolItem.h"  // for AbstractToolItem

class SpinPageAdapter;
class ActionHandler;

class ToolPageSpinner: public AbstractToolItem {
public:
    ToolPageSpinner(ActionHandler* handler, std::string id, ActionType type, IconNameHelper iconNameHelper);
    ~ToolPageSpinner() override;

public:
    SpinPageAdapter* getPageSpinner() const;
    void setPageInfo(size_t pagecount, size_t pdfpage);
    std::string getToolDisplayName() const override;
    GtkToolItem* createItem(bool horizontal) override;
    GtkToolItem* createTmpItem(bool horizontal) override;

protected:
    GtkToolItem* newItem() override;
    GtkWidget* getNewToolIcon() const override;
    GdkPixbuf* getNewToolPixbuf() const override;

private:
    void updateLabels();

private:
    SpinPageAdapter* pageSpinner = nullptr;
    GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;

    GtkWidget* box = nullptr;
    GtkWidget* lbPageNo = nullptr;
    GtkWidget* lbVerticalPdfPage = nullptr;

    /** The current page of the document. */
    size_t pageCount = 0;

    /** The current page in the background PDF, or 0 if there is no PDF. */
    size_t pdfPage = 0;

    IconNameHelper iconNameHelper;
};
