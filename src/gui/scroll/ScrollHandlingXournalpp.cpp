#include "ScrollHandlingXournalpp.h"


ScrollHandlingXournalpp::ScrollHandlingXournalpp()
: ScrollHandling(gtk_adjustment_new(0, 0, 100, 5, 10, 10), gtk_adjustment_new(0, 0, 100, 5, 10, 10))
{
	XOJ_INIT_TYPE(ScrollHandlingXournalpp);
}

ScrollHandlingXournalpp::~ScrollHandlingXournalpp()
{
	XOJ_RELEASE_TYPE(ScrollHandlingXournalpp);
}

void ScrollHandlingXournalpp::setLayoutSize(int width, int height)
{
	XOJ_CHECK_TYPE(ScrollHandlingXournalpp);

	// TODO !!!!!!!!!!!
}

int ScrollHandlingXournalpp::getPrefferedWidth()
{
	XOJ_CHECK_TYPE(ScrollHandlingXournalpp);

	return 400;
}

int ScrollHandlingXournalpp::getPrefferedHeight()
{
	XOJ_CHECK_TYPE(ScrollHandlingXournalpp);

	return 400;
}
