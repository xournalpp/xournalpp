#include "ScrollHandling.h"


ScrollHandling::ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical)
 : adjHorizontal(adjHorizontal),
   adjVertical(adjVertical)
{
}

ScrollHandling::~ScrollHandling()
{
}

GtkAdjustment* ScrollHandling::getHorizontal()
{
	return adjHorizontal;
}

GtkAdjustment* ScrollHandling::getVertical()
{
	return adjVertical;
}

void ScrollHandling::init(GtkWidget* xournal, Layout* layout)
{
	this->xournal = xournal;
	this->layout = layout;
}
