#include "RepaintHandler.h"

#include "PageView.h"
#include "XournalView.h"

#include "widgets/XournalWidget.h"

RepaintHandler::RepaintHandler(XournalView* xournal)
{
	XOJ_INIT_TYPE(RepaintHandler);

	this->xournal = xournal;
}

RepaintHandler::~RepaintHandler()
{
	XOJ_CHECK_TYPE(RepaintHandler);

	this->xournal = NULL;

	XOJ_RELEASE_TYPE(RepaintHandler);
}

void RepaintHandler::repaintPage(XojPageView* view)
{
	XOJ_CHECK_TYPE(RepaintHandler);

//	int x1 = view->getX();
//	int y1 = view->getY();
//	int x2 = x1 + view->getDisplayWidth();
//	int y2 = y1 + view->getDisplayHeight();
//	gtk_xournal_repaint_area(this->xournal->getWidget(), x1, y1, x2, y2);

	// Repainting is really fast, and there are painting issues from time to time
	// Therefore do always a full repaint. The stroken are cached anyway.
	// Tested on a 1.8GHz Core2Duo
	gtk_widget_queue_draw(this->xournal->getWidget());
}

void RepaintHandler::repaintPageArea(XojPageView* view, int x1, int y1, int x2, int y2)
{
	XOJ_CHECK_TYPE(RepaintHandler);

//	int x = view->getX();
//	int y = view->getY();
//	gtk_xournal_repaint_area(this->xournal->getWidget(), x + x1, y + y1, x + x2, y + y2);

	// Repainting is really fast, and there are painting issues from time to time
	// Therefore do always a full repaint. The stroken are cached anyway.
	// Tested on a 1.8GHz Core2Duo
	gtk_widget_queue_draw(this->xournal->getWidget());
}

void RepaintHandler::repaintPageBorder(XojPageView* view)
{
	XOJ_CHECK_TYPE(RepaintHandler);

	gtk_widget_queue_draw(this->xournal->getWidget());
}
