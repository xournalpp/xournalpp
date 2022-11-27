#include "PdfFloatingToolbox.h"

#include <algorithm>  // for max, min
#include <cmath>      // for abs
#include <string>     // for string
#include <utility>    // for move
#include <vector>     // for vector

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
    g_signal_connect(theMainWindow->get("pdfTbChangeType"), "clicked", G_CALLBACK(this->switchSelectTypeCb), this);

    this->clearSelection();
    this->hide();
}

PdfFloatingToolbox::~PdfFloatingToolbox() = default;

PdfElemSelection* PdfFloatingToolbox::getSelection() const { return this->pdfElemSelection.get(); }
bool PdfFloatingToolbox::hasSelection() const { return this->getSelection() != nullptr; }

void PdfFloatingToolbox::clearSelection() { this->pdfElemSelection.reset(); }

auto PdfFloatingToolbox::newSelection(double x, double y) -> const PdfElemSelection* {
    this->pdfElemSelection = std::make_unique<PdfElemSelection>(x, y, this->theMainWindow->getControl());
    return this->pdfElemSelection.get();
}

void PdfFloatingToolbox::show(int x, int y) {
    g_assert_nonnull(this->getSelection());

    // (x, y) are in the gtk window's coordinates.
    // However, we actually show the toolbox in the overlay's coordinate system.
    gtk_widget_translate_coordinates(gtk_widget_get_toplevel(this->floatingToolbox), GTK_WIDGET(overlay.get()), x, y,
                                     &this->position.x, &this->position.y);
    this->show();
}

void PdfFloatingToolbox::hide() {
    if (isHidden())
        return;

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

        GtkAllocation windowAlloc{};
        gtk_widget_get_allocation(GTK_WIDGET(overlay), &windowAlloc);

        bool rightOK = self->position.x + allocation->width + gap <= windowAlloc.width;
        bool bottomOK = self->position.y + allocation->height + gap <= windowAlloc.height;

        allocation->x = rightOK ? self->position.x + gap : self->position.x - allocation->width - gap;
        allocation->y = bottomOK ? self->position.y + gap : self->position.y - allocation->height - gap;

        return true;
    }

    return false;
}

void PdfFloatingToolbox::userCancelSelection() {
    this->pdfElemSelection.reset();
    this->hide();
}

void PdfFloatingToolbox::highlightCb(GtkButton* button, PdfFloatingToolbox* pft) {
    pft->createStrokes(PdfMarkerStyle::POS_TEXT_MIDDLE, PdfMarkerStyle::WIDTH_TEXT_HEIGHT, 60);
    pft->userCancelSelection();
}

void PdfFloatingToolbox::copyTextCb(GtkButton* button, PdfFloatingToolbox* pft) {
    pft->copyTextToClipboard();
    pft->userCancelSelection();
}

void PdfFloatingToolbox::underlineCb(GtkButton* button, PdfFloatingToolbox* pft) {
    pft->createStrokes(PdfMarkerStyle::POS_TEXT_BOTTOM, PdfMarkerStyle::WIDTH_TEXT_LINE, 230);
    pft->userCancelSelection();
}

void PdfFloatingToolbox::strikethroughCb(GtkButton* button, PdfFloatingToolbox* pft) {
    pft->createStrokes(PdfMarkerStyle::POS_TEXT_MIDDLE, PdfMarkerStyle::WIDTH_TEXT_LINE, 230);
    pft->userCancelSelection();
}

void PdfFloatingToolbox::show() {
    gtk_widget_hide(this->floatingToolbox);  // force showing in new position
    gtk_widget_show_all(this->floatingToolbox);
}

void PdfFloatingToolbox::copyTextToClipboard() {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->theMainWindow->getWindow(), GDK_SELECTION_CLIPBOARD);
    if (std::string text = this->pdfElemSelection->getSelectedText(); !text.empty()) {
        gtk_clipboard_set_text(clipboard, text.c_str(), -1);
    }
}

void PdfFloatingToolbox::createStrokes(PdfMarkerStyle position, PdfMarkerStyle width, int markerOpacity) {
    const uint64_t pdfPageNo = this->pdfElemSelection->getSelectionPageNr();
    const uint64_t currentPage = theMainWindow->getXournal()->getCurrentPage();

    // Get the PDF page that the current page corresponds to.
    // It should be the same as the PDF page of the selection.
    auto doc = this->theMainWindow->getControl()->getDocument();
    doc->lock();
    const uint64_t pdfPageOfCurrentPage = doc->getPage(currentPage)->getPdfPageNr();
    doc->unlock();

    if (pdfPageOfCurrentPage != pdfPageNo) {
        // There's probably a bug that violates our assumptions, so no-op.
        g_warning("The current page's PDF page is not the same as the PDF page of the selection!");
        return;
    }

    const auto textRects = this->pdfElemSelection->getSelectedTextRects();
    if (textRects.empty()) {
        return;
    }

    auto* control = this->theMainWindow->getControl();
    PageRef page = control->getCurrentPage();
    Layer* layer = page->getSelectedLayer();

    auto color = theMainWindow->getXournal()->getControl()->getToolHandler()->getColor();

    Range dirtyRange;
    std::vector<Element*> strokes;
    for (XojPdfRectangle rect: textRects) {
        const double topOfLine = std::min(rect.y1, rect.y2);
        const double middleOfLine = (rect.y1 + rect.y2) / 2;
        const double bottomOfLine = std::max(rect.y1, rect.y2);
        const double rectWidth = std::abs(rect.y2 - rect.y1);

        // the center line position of stroke
        const double h = position == PdfMarkerStyle::POS_TEXT_BOTTOM ? bottomOfLine :
                         position == PdfMarkerStyle::POS_TEXT_MIDDLE ? middleOfLine :
                                                                       topOfLine;
        // the width of stroke
        const double w = width == PdfMarkerStyle::WIDTH_TEXT_LINE ? 1 : rectWidth;

        auto* stroke = new Stroke();
        stroke->setColor(color);
        stroke->setFill(markerOpacity);
        stroke->setToolType(StrokeTool::HIGHLIGHTER);
        stroke->setWidth(w);
        stroke->addPoint(Point(rect.x1, h, -1));
        stroke->addPoint(Point(rect.x2, h, -1));
        stroke->setStrokeCapStyle(StrokeCapStyle::BUTT);

        dirtyRange.addPoint(rect.x1, h - 0.5 * w);
        dirtyRange.addPoint(rect.x2, h + 0.5 * w);

        strokes.push_back(stroke);
    }

    doc->lock();
    for (auto* s: strokes) {
        layer->addElement(s);
    }
    doc->unlock();
    page->fireElementsChanged(strokes, dirtyRange);

    auto undoAct = std::make_unique<GroupUndoAction>();
    for (auto* stroke: strokes) {
        undoAct->addAction(std::make_unique<InsertUndoAction>(page, layer, stroke));
    }
    control->getUndoRedoHandler()->addUndoAction(std::move(undoAct));
}

void PdfFloatingToolbox::switchSelectTypeCb(GtkButton* button, PdfFloatingToolbox* pft) {
    ToolType type = pft->theMainWindow->getControl()->getToolHandler()->getToolType();

    type = type == ToolType::TOOL_SELECT_PDF_TEXT_LINEAR ? ToolType::TOOL_SELECT_PDF_TEXT_RECT :
                                                           ToolType::TOOL_SELECT_PDF_TEXT_LINEAR;

    pft->theMainWindow->getControl()->selectTool(type);

    pft->pdfElemSelection->setToolType(type);
    pft->pdfElemSelection->finalizeSelection(PdfElemSelection::selectionStyleForToolType(type));
}

bool PdfFloatingToolbox::isHidden() const { return !gtk_widget_is_visible(this->floatingToolbox); }
