#include "PdfFloatingToolbox.h"

#include <algorithm>  // for max, min
#include <cmath>      // for abs
#include <cstddef>    // for size_t
#include <memory>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <gtk/gtk.h>

#include "control/Control.h"      // for Control
#include "control/ToolEnums.h"    // for ToolType, TOOL_SELECT_PDF_TEXT_LI...
#include "control/ToolHandler.h"  // for ToolHandler
#include "control/tools/PdfElemSelection.h"
#include "gui/PageView.h"           // for XojPageView
#include "gui/XournalView.h"        // for XournalView
#include "model/Document.h"         // for Document
#include "model/Layer.h"            // for Layer
#include "model/PageRef.h"          // for PageRef
#include "model/Point.h"            // for Point
#include "model/Stroke.h"           // for Stroke, BUTT, StrokeTool::HIGHLIG...
#include "model/XojPage.h"          // for XojPage
#include "undo/GroupUndoAction.h"   // for GroupUndoAction
#include "undo/InsertUndoAction.h"  // for InsertUndoAction
#include "undo/UndoAction.h"        // for UndoAction
#include "undo/UndoRedoHandler.h"   // for UndoRedoHandler
#include "util/Assert.h"            // for xoj_assert
#include "util/gtk4_helper.h"       // for gtk_widget_get_clipboard

#include "MainWindow.h"  // for MainWindow

PdfFloatingToolbox::PdfFloatingToolbox(MainWindow* theMainWindow, GtkOverlay* overlay):
        theMainWindow(theMainWindow), overlay(overlay, xoj::util::ref), position({0, 0}) {
    this->floatingToolbox = theMainWindow->get("pdfFloatingToolbox");

    gtk_overlay_add_overlay(overlay, this->floatingToolbox);
    gtk_overlay_set_overlay_pass_through(overlay, this->floatingToolbox, true);

    g_signal_connect(overlay, "get-child-position", G_CALLBACK(this->getOverlayPosition), this);

    g_signal_connect(theMainWindow->get("pdfTbHighlight"), "clicked", G_CALLBACK(this->highlightCb), this);
    g_signal_connect(theMainWindow->get("pdfTbCopyText"), "clicked", G_CALLBACK(this->copyTextCb), this);
    g_signal_connect(theMainWindow->get("pdfTbUnderline"), "clicked", G_CALLBACK(this->underlineCb), this);
    g_signal_connect(theMainWindow->get("pdfTbStrikethrough"), "clicked", G_CALLBACK(this->strikethroughCb), this);

    this->hide();
}

PdfFloatingToolbox::~PdfFloatingToolbox() = default;

void PdfFloatingToolbox::show(int x, int y, PdfElemSelection* selection) {
    this->selection = selection;
    this->position = {x, y};
    this->show();
}

void PdfFloatingToolbox::hide() {
    this->selection = nullptr;
    gtk_widget_hide(this->floatingToolbox);
}

auto PdfFloatingToolbox::getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                            PdfFloatingToolbox* self) -> gboolean {
    if (widget == self->floatingToolbox) {
        // Get existing width and height
        GtkRequisition natural;
        gtk_widget_get_preferred_size(widget, nullptr, &natural);
        allocation->width = natural.width;
        allocation->height = natural.height;

        // Make sure the "pdfFloatingToolbox" is fully displayed.
        const int gap = 5;

        // By default, we show the toolbox below and to the right of the selected text.
        // If the toolbox will go out of the window, then we'll flip the corresponding directions.

        GtkWidget* scrolledWindow =
                gtk_widget_get_ancestor(self->theMainWindow->getXournal()->getWidget(), GTK_TYPE_SCROLLED_WINDOW);

        bool rightOK = self->position.x + allocation->width + gap <= gtk_widget_get_allocated_width(scrolledWindow);
        bool bottomOK = self->position.y + allocation->height + gap <= gtk_widget_get_allocated_height(scrolledWindow);

        allocation->x = rightOK ? self->position.x + gap : self->position.x - allocation->width - gap;
        allocation->y = bottomOK ? self->position.y + gap : self->position.y - allocation->height - gap;

        gtk_widget_translate_coordinates(scrolledWindow, GTK_WIDGET(overlay), allocation->x, allocation->y,
                                         &allocation->x, &allocation->y);

        return true;
    }

    return false;
}

template <auto fun>
void PdfFloatingToolbox::cb() {
    if (!this->selection) {
        g_warning("PdfFloatingToolbox: Callback without selection");
        return;
    }
    (this->selection->*fun)();
    this->hide();
}

void PdfFloatingToolbox::highlightCb(GtkButton*, PdfFloatingToolbox* self) { self->cb<&PdfElemSelection::highlight>(); }

void PdfFloatingToolbox::underlineCb(GtkButton*, PdfFloatingToolbox* self) { self->cb<&PdfElemSelection::underline>(); }

void PdfFloatingToolbox::strikethroughCb(GtkButton*, PdfFloatingToolbox* self) {
    self->cb<&PdfElemSelection::strikethrough>();
}

void PdfFloatingToolbox::copyTextCb(GtkButton*, PdfFloatingToolbox* self) { self->cb<&PdfElemSelection::copy>(); }

void PdfFloatingToolbox::show() {
    gtk_widget_hide(this->floatingToolbox);  // force showing in new position
    gtk_widget_show_all(this->floatingToolbox);
}
