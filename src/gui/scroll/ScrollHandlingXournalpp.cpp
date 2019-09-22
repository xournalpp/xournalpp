#include "ScrollHandlingXournalpp.h"


ScrollHandlingXournalpp::ScrollHandlingXournalpp()
: ScrollHandling(gtk_adjustment_new(0, 0, 100, 5, 10, 10), gtk_adjustment_new(0, 0, 100, 5, 10, 10))
{
}

ScrollHandlingXournalpp::~ScrollHandlingXournalpp()
{
}

void ScrollHandlingXournalpp::setLayoutSize(int width, int height)
{
	gtk_adjustment_set_page_size(adjHorizontal, gtk_widget_get_allocated_width(xournal));
	gtk_adjustment_set_upper(adjHorizontal, width);

	gtk_adjustment_set_page_size(adjVertical, gtk_widget_get_allocated_height(xournal));
	gtk_adjustment_set_upper(adjVertical, height);
}

int ScrollHandlingXournalpp::getPreferredWidth()
{
	return 400;
}

int ScrollHandlingXournalpp::getPreferredHeight()
{
	return 400;
}

void ScrollHandlingXournalpp::translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2)
{
	double h = gtk_adjustment_get_value(adjHorizontal);
	double v = gtk_adjustment_get_value(adjVertical);
	cairo_translate(cr, -h, -v);

	x1 += h;
	x2 += h;

	y1 += v;
	y2 += v;
}

void ScrollHandlingXournalpp::translate(double& x, double& y)
{
	double h = gtk_adjustment_get_value(adjHorizontal);
	double v = gtk_adjustment_get_value(adjVertical);

	x += h;
	y += v;
}

bool ScrollHandlingXournalpp::fullRepaint()
{
	return true;
}

void ScrollHandlingXournalpp::scrollChanged()
{
	gtk_widget_queue_draw(xournal);
}

