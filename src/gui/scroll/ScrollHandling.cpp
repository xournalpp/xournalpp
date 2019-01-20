#include "ScrollHandling.h"


ScrollHandling::ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical)
 : adjHorizontal(adjHorizontal),
   adjVertical(adjVertical)
{
	XOJ_INIT_TYPE(ScrollHandling);
}

ScrollHandling::~ScrollHandling()
{
	XOJ_RELEASE_TYPE(ScrollHandling);
}

GtkAdjustment* ScrollHandling::getHorizontal()
{
	XOJ_CHECK_TYPE(ScrollHandling);

	return adjHorizontal;
}

GtkAdjustment* ScrollHandling::getVertical()
{
	XOJ_CHECK_TYPE(ScrollHandling);

	return adjVertical;
}

void ScrollHandling::init(GtkWidget* xournal, Layout* layout)
{
	this->xournal = xournal;
	this->layout = layout;
}
