#include "RepaintHandler.h"

#include <gtk/gtk.h>  // for gtk_widget_queue_draw

#include "gui/widgets/XournalWidget.h"  // for gtk_xournal_repaint_area

#include "PageView.h"     // for XojPageView
#include "XournalView.h"  // for XournalView

RepaintHandler::RepaintHandler(XournalView* xournal): xournal(xournal) {}

RepaintHandler::~RepaintHandler() { this->xournal = nullptr; }

void RepaintHandler::repaintPage(const XojPageView* view) {
    auto p = view->getPixelPosition();
    int x2 = p.x + view->getDisplayWidth();
    int y2 = p.y + view->getDisplayHeight();
    gtk_xournal_repaint_area(this->xournal->getWidget(), p.x, p.y, x2, y2);
}

void RepaintHandler::repaintPageArea(const XojPageView* view, int x1, int y1, int x2, int y2) {
    auto p = view->getPixelPosition();
    gtk_xournal_repaint_area(this->xournal->getWidget(), p.x + x1, p.y + y1, p.x + x2, p.y + y2);
}

void RepaintHandler::repaintPageBorder(const XojPageView* view) { gtk_widget_queue_draw(this->xournal->getWidget()); }
