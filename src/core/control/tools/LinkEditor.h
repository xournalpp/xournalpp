/*
 * Xournal++
 *
 * Link editor gui (for Link Tool)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <gdk/gdk.h>  // for GdkEventKey
#include <gtk/gtk.h>  // for GtkIMContext, GtkTextIter, GtkWidget

#include "model/Link.h"     // for Link
#include "model/PageRef.h"  // for PageRef

class Control;

class LinkEditor {
public:
    LinkEditor(Control* control, GtkWidget* xournalWidget);
    ~LinkEditor();

    void startEditing(const PageRef& page, const int x, const int y, const bool controlDown);

private:
    Control* control;
    GtkWidget* documentWidget;
    Link* linkElement = nullptr;
};
