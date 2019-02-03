#include "Layout.h"

#include "XournalView.h"

#include "control/Control.h"
#include "pageposition/PagePositionHandler.h"
#include "widgets/XournalWidget.h"
#include "gui/scroll/ScrollHandling.h"


Layout::Layout(XournalView* view, ScrollHandling* scrollHandling)
 : view(view),
   scrollHandling(scrollHandling),
   lastScrollHorizontal(-1),
   lastScrollVertical(-1),
   lastWidgetWidth(0),
   layoutWidth(0),
   layoutHeight(0)
{
	XOJ_INIT_TYPE(Layout);

	g_signal_connect(scrollHandling->getHorizontal(), "value-changed", G_CALLBACK(
		+[](GtkAdjustment* adjustment, Layout* layout)
		{
			XOJ_CHECK_TYPE_OBJ(layout, Layout);
			layout->checkScroll(adjustment, layout->lastScrollHorizontal);
			layout->updateCurrentPage();
			layout->scrollHandling->scrollChanged();
		}), this);

	g_signal_connect(scrollHandling->getVertical(), "value-changed", G_CALLBACK(
		+[](GtkAdjustment* adjustment, Layout* layout)
		{
			XOJ_CHECK_TYPE_OBJ(layout, Layout);
			layout->checkScroll(adjustment, layout->lastScrollVertical);
			layout->updateCurrentPage();
			layout->scrollHandling->scrollChanged();
		}), this);

	lastScrollHorizontal = gtk_adjustment_get_value(scrollHandling->getHorizontal());
	lastScrollVertical = gtk_adjustment_get_value(scrollHandling->getVertical());
}

Layout::~Layout()
{
	XOJ_RELEASE_TYPE(Layout);
}

void Layout::checkScroll(GtkAdjustment* adjustment, double& lastScroll)
{
	XOJ_CHECK_TYPE(Layout);

	lastScroll = gtk_adjustment_get_value(adjustment);
}

/**
 * Check which page should be selected
 */
void Layout::updateCurrentPage()
{
	XOJ_CHECK_TYPE(Layout);

	Rectangle visRect = getVisibleRect();

	Control* control = this->view->getControl();

	bool twoPages = control->getSettings()->isShowTwoPages();

	if (visRect.y < 1)
	{
		if (twoPages && this->view->viewPagesLen > 1 &&
		    this->view->viewPages[1]->isSelected())
		{
			// page 2 already selected
		}
		else
		{
			control->firePageSelected(0);
		}
		return;
	}

	size_t mostPageNr = 0;
	double mostPagePercent = 0;

	for (size_t page = 0; page < this->view->viewPagesLen; page++)
	{
		XojPageView* p = this->view->viewPages[page];

		Rectangle currentRect = p->getRect();

		// if we are already under the visible rectangle
		// then everything below will not be visible...
		if(currentRect.y > visRect.y + visRect.height)
		{
			p->setIsVisible(false);
			for (; page < this->view->viewPagesLen; page++)
			{
				p = this->view->viewPages[page];
				p->setIsVisible(false);
			}

			break;
		}

		// if the condition is satisfied we know that
		// the rectangles intersect vertically
		if (currentRect.y + currentRect.height >= visRect.y)
		{

			double percent =
				currentRect.intersect(visRect).area() / currentRect.area();

			if (percent > mostPagePercent)
			{
				mostPagePercent = percent;
				mostPageNr = page;
			}

			p->setIsVisible(true);
		}
		else
		{
			p->setIsVisible(false);
		}
	}

	if (twoPages && mostPageNr < this->view->viewPagesLen - 1)
	{
		int y1 = this->view->viewPages[mostPageNr]->getY();
		int y2 = this->view->viewPages[mostPageNr + 1]->getY();

		if (y1 != y2 || !this->view->viewPages[mostPageNr + 1]->isSelected())
		{
			// if the second page is selected DON'T select the first page.
			// Only select the first page if none is selected
			control->firePageSelected(mostPageNr);
		}
	}
	else
	{
		control->firePageSelected(mostPageNr);
	}
}

Rectangle Layout::getVisibleRect()
{
	XOJ_CHECK_TYPE(Layout);

	return Rectangle(gtk_adjustment_get_value(scrollHandling->getHorizontal()),
	                 gtk_adjustment_get_value(scrollHandling->getVertical()),
	                 gtk_adjustment_get_page_size(scrollHandling->getHorizontal()),
	                 gtk_adjustment_get_page_size(scrollHandling->getVertical()));
}

/**
 * Returns the height of the entire Layout
 */
double Layout::getLayoutHeight()
{
	XOJ_CHECK_TYPE(Layout);

	return layoutHeight;
}

/**
 * Returns the width of the entire Layout
 */
double Layout::getLayoutWidth()
{
	XOJ_CHECK_TYPE(Layout);

	return layoutWidth;
}

/**
 * Padding outside the pages, including shadow
 */
const int XOURNAL_PADDING = 10;

/**
 * Padding between the pages
 */
const int XOURNAL_PADDING_BETWEEN = 15;



/**
 * decides whether we use book mode, when dual page is active or not
 * currently unused; remove the 3 pragmas after implementing left aligned page start
 */
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCSimplifyInspection"
constexpr bool second_p_start = true;


void Layout::layoutPages()
{
	XOJ_CHECK_TYPE(Layout);

	size_t len = this->view->viewPagesLen;

	Settings *settings = this->view->getControl()->getSettings();
	bool verticalSpace = settings->getAddVerticalSpace();
	bool horizontalSpace = settings->getAddHorizontalSpace();
	bool dualPage = settings->isShowTwoPages();
//   dualPage &= len > 1;
	// calculate maximum size
	int size[2] = {0, 0};
	for (size_t i = 0; i < len; i++)
	{
		XojPageView *v = this->view->viewPages[i];

		size_t rId = dualPage ? ((i % 2u) + (second_p_start ? 1u : 0u)) % 2u : 0u;

		size[rId] = std::max(size[rId], v->getDisplayWidth());
	}
	if (len == 1)
	{
		size[0] = size[1] = std::max(size[0], size[1]);
	}

	int marginLeft = XOURNAL_PADDING;
	int marginTop = XOURNAL_PADDING;
	int y = marginTop;

	int width = size[0] + 2 * XOURNAL_PADDING;
	if (dualPage)
	{
		width += XOURNAL_PADDING_BETWEEN + size[1];
	}

	auto visRect = getVisibleRect();

	marginLeft = std::max(marginLeft, (int) ((visRect.width - width) / 2.f));

	if (horizontalSpace)
	{
		//A quarter of the document is always visible in window
		marginLeft = std::max(marginLeft, (int) (visRect.width * 0.75));
	}

	if (verticalSpace)
	{
		marginTop = std::max(marginTop, (int) (visRect.height * 0.75));
	}

	int last_height = 0;
	for (int i = 0; i < len; i++)
	{
		auto v = this->view->viewPages[i];
		bool second = (i % 2 != 0) ^ second_p_start;
		int x;

		/*
		 * Center pages vertically, like this
		 *  single          double
		 *
		 * |  [=]  |      | [=][=]  |
		 * | [===] |      |[==][===]|
		 * | [===] |      |[==][==] |
		 * |  [=]  |      | [=][===]|
		 */

		if (!dualPage)
		{
			x = (size[0] - v->getDisplayWidth() + 1) / 2 + XOURNAL_PADDING;
		}
		else
		{
			if (!second)
			{
				x = size[0] - v->getDisplayWidth() + XOURNAL_PADDING;
			}
			else
			{
				x = size[0] + XOURNAL_PADDING_BETWEEN + XOURNAL_PADDING;
			}
		}

		v->layout.setX(x);
		v->layout.setY(y);
		v->layout.setMarginLeft(marginLeft);
		v->layout.setMarginTop(marginTop);

		last_height = dualPage && second ? std::max(last_height, v->getDisplayHeight()) : v->getDisplayHeight();
		if (!dualPage || second)
		{
			y += XOURNAL_PADDING_BETWEEN + last_height;
			last_height = 0;
		}
	}
	y += last_height + 2 * marginTop + XOURNAL_PADDING - XOURNAL_PADDING_BETWEEN;
	width += 2 * marginLeft;

	this->setLayoutSize(width, y);
	this->view->pagePosition->update(this->view->viewPages, len, y);
}
#pragma clang diagnostic pop

void Layout::setLayoutSize(int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	this->layoutHeight = height;
	this->layoutWidth = width;

	this->scrollHandling->setLayoutSize(width, height);
}

void Layout::setSize(int widgetWidth, int widgetHeight)
{
	XOJ_CHECK_TYPE(Layout);

	if (this->lastWidgetWidth != widgetWidth)
	{
		this->layoutPages();
		this->lastWidgetWidth = widgetWidth;
	}
	else
	{
		this->setLayoutSize(this->layoutWidth, this->layoutHeight);
	}

	this->view->getControl()->calcZoomFitSize();
}

void Layout::scrollRelativ(int x, int y)
{
	XOJ_CHECK_TYPE(Layout);

	gtk_adjustment_set_value(scrollHandling->getHorizontal(), gtk_adjustment_get_value(scrollHandling->getHorizontal()) + x);
	gtk_adjustment_set_value(scrollHandling->getVertical(), gtk_adjustment_get_value(scrollHandling->getVertical()) + y);
}

void Layout::scrollAbs(int x, int y)
{
	XOJ_CHECK_TYPE(Layout);

	gtk_adjustment_set_value(scrollHandling->getHorizontal(), x);
	gtk_adjustment_set_value(scrollHandling->getVertical(), y);
}


void Layout::ensureRectIsVisible(int x, int y, int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	gtk_adjustment_clamp_page(scrollHandling->getHorizontal(), x - 5, x + width + 10);
	gtk_adjustment_clamp_page(scrollHandling->getVertical(), y - 5, y + height + 10);
}



