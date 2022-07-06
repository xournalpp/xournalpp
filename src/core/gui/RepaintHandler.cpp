#include "RepaintHandler.h"

#include <gtk/gtk.h>  // for gtk_widget_queue_draw

#include "gui/widgets/XournalWidget.h"  // for gtk_xournal_repaint_area

#include "PageView.h"     // for XojPageView
#include "XournalView.h"  // for XournalView

RepaintHandler::RepaintHandler(XournalView* xournal): xournal(xournal) {}

RepaintHandler::~RepaintHandler() { this->xournal = nullptr; }

void RepaintHandler::repaintPage(const XojPageView* view) {
    int x1 = view->getX();
    int y1 = view->getY();
    int x2 = x1 + view->getDisplayWidth();
    int y2 = y1 + view->getDisplayHeight();

    gtk_xournal_repaint_area(this->xournal->getWidget(), x1, y1, x2, y2);
}

void RepaintHandler::repaintPageArea(const XojPageView* view, int x1, int y1, int x2, int y2) {
    int x = view->getX();
    int y = view->getY();
    gtk_xournal_repaint_area(this->xournal->getWidget(), x + x1, y + y1, x + x2, y + y2);
}

void RepaintHandler::repaintPageBorder(const XojPageView* view) { gtk_widget_queue_draw(this->xournal->getWidget()); }
