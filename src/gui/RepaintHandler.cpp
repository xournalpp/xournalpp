#include "RepaintHandler.h"

#include "gui/scroll/ScrollHandling.h"
#include "widgets/XournalWidget.h"

#include "PageView.h"
#include "XournalView.h"

RepaintHandler::RepaintHandler(XournalView* xournal): xournal(xournal) {}

RepaintHandler::~RepaintHandler() { this->xournal = nullptr; }

void RepaintHandler::repaintPage(XojPageView* view) {
    if (xournal->getScrollHandling()->fullRepaint()) {
        gtk_widget_queue_draw(this->xournal->getWidget()->getGtkWidget());
    } else {
        int x1 = view->getX();
        int y1 = view->getY();
        int x2 = x1 + view->getDisplayWidth();
        int y2 = y1 + view->getDisplayHeight();

        this->xournal->getWidget()->repaintArea(x1, y1, x2, y2);
    }
}

void RepaintHandler::repaintPageArea(XojPageView* view, int x1, int y1, int x2, int y2) {
    if (xournal->getScrollHandling()->fullRepaint()) {
        gtk_widget_queue_draw(this->xournal->getWidget()->getGtkWidget());
    } else {
        int x = view->getX();
        int y = view->getY();
        this->xournal->getWidget()->repaintArea(x + x1, y + y1, x + x2, y + y2);
    }
}

void RepaintHandler::repaintPageBorder(XojPageView* view) {
    gtk_widget_queue_draw(this->xournal->getWidget()->getGtkWidget());
}
