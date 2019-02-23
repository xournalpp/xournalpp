#include "RepaintHandler.h"

#include "PageView.h"
#include "XournalView.h"

#include "gui/scroll/ScrollHandling.h"
#include "widgets/XournalWidget.h"

RepaintHandler::RepaintHandler(XournalView* xournal)
 : xournal(xournal)
{
	XOJ_INIT_TYPE(RepaintHandler);
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

	if (xournal->getScrollHandling()->fullRepaint())
	{
		gtk_widget_queue_draw(this->xournal->getWidget());
	}
	else
	{
		int x1 = view->getX();
		int y1 = view->getY();
		int x2 = x1 + view->getDisplayWidth();
		int y2 = y1 + view->getDisplayHeight();

		gtk_xournal_repaint_area(this->xournal->getWidget(), x1, y1, x2, y2);
	}
}

void RepaintHandler::repaintPageArea(XojPageView* view, int x1, int y1, int x2, int y2)
{
	XOJ_CHECK_TYPE(RepaintHandler);


	if (xournal->getScrollHandling()->fullRepaint())
	{
		gtk_widget_queue_draw(this->xournal->getWidget());
	}
	else
	{
		int x = view->getX();
		int y = view->getY();
		gtk_xournal_repaint_area(this->xournal->getWidget(), x + x1, y + y1, x + x2, y + y2);
	}
}

void RepaintHandler::repaintPageBorder(XojPageView* view)
{
	XOJ_CHECK_TYPE(RepaintHandler);

	gtk_widget_queue_draw(this->xournal->getWidget());
}
