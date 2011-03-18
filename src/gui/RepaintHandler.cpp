#include "RepaintHandler.h"
#include "XournalView.h"
#include "widgets/XournalWidget.h"
#include "PageView.h"

RepaintHandler::RepaintHandler(XournalView * xournal) {
	this->xournal = xournal;
}

RepaintHandler::~RepaintHandler() {
	this->xournal = NULL;
}

void RepaintHandler::repaintPage(PageView * view) {
	int x1 = view->getX();
	int y1 = view->getY();
	int x2 = view->getX() + view->getDisplayWidth();
	int y2 = view->getY() + view->getDisplayHeight();
	gtk_xournal_repaint_area(this->xournal->getWidget(), x1, y1, x2, y2);
}

void RepaintHandler::repaintPageArea(PageView * view, int x1, int y1, int x2, int y2) {
	gtk_xournal_repaint_area(this->xournal->getWidget(), x1, y1, x2, y2);
}

void RepaintHandler::repaintPageBorder(PageView * view) {
	gtk_widget_queue_draw(this->xournal->getWidget());
}
