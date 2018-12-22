#include "Layout.h"

#include "XournalView.h"

#include "control/Control.h"
#include "pageposition/PagePositionHandler.h"
#include "widgets/XournalWidget.h"


Layout::Layout(XournalView* view,
               GtkAdjustment* adjHorizontal,
	           GtkAdjustment* adjVertical)
 : view(view),
   adjHorizontal(adjHorizontal),
   adjVertical(adjVertical),
   lastScrollHorizontal(-1),
   lastScrollVertical(-1),
   lastWidgetWidth(0),
   layoutWidth(0),
   layoutHeight(0)
{
	XOJ_INIT_TYPE(Layout);

	g_signal_connect(adjHorizontal, "value-changed", G_CALLBACK(
		+[](GtkAdjustment* adjustment, Layout* layout)
		{
			XOJ_CHECK_TYPE_OBJ(layout, Layout);
			layout->checkScroll(adjustment, layout->lastScrollHorizontal);
			layout->updateCurrentPage();
		}), this);

	g_signal_connect(adjVertical, "value-changed", G_CALLBACK(
		+[](GtkAdjustment* adjustment, Layout* layout)
		{
			XOJ_CHECK_TYPE_OBJ(layout, Layout);
			layout->checkScroll(adjustment, layout->lastScrollVertical);
			layout->updateCurrentPage();
		}), this);

	lastScrollHorizontal = gtk_adjustment_get_value(adjHorizontal);
	lastScrollVertical = gtk_adjustment_get_value(adjVertical);
}

Layout::~Layout()
{
	XOJ_RELEASE_TYPE(Layout);
}

void Layout::checkScroll(GtkAdjustment* adjustment, double& lastScroll)
{
	XOJ_CHECK_TYPE(Layout);

	lastScroll = gtk_adjustment_get_value(adjustment);

/**
 * This causes issues.
 *
 * Disabling devices does not work
 * Removing Touch mask does not work
 *
 * So an X11 / Wayland etc. implementation should be created
 */
//	ToolHandler* h = view->getControl()->getToolHandler();
//
//	if (!view->shouldIgnoreTouchEvents() || h->getToolType() == TOOL_HAND)
//	{
//		lastScroll = gtk_adjustment_get_value(adjustment);
//		return;
//	}
//	gtk_adjustment_set_value(adjustment, lastScroll);
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

	return Rectangle(gtk_adjustment_get_value(adjHorizontal),
	                 gtk_adjustment_get_value(adjVertical),
	                 gtk_adjustment_get_page_size(adjHorizontal),
	                 gtk_adjustment_get_page_size(adjVertical));
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

void Layout::layoutPages()
{
	XOJ_CHECK_TYPE(Layout);

	int y = 0;

	int len = this->view->viewPagesLen;

	Settings* settings = this->view->getControl()->getSettings();
	bool verticalSpace = settings->getAddVerticalSpace();
	bool horizontalSpace = settings->getAddHorizontalSpace();
	bool dualPage = settings->isShowTwoPages();

	int size[2] = { 0, 0 };

	// we need at least 2 page for dual page view
	if (len < 2)
	{
		dualPage = false;
	}

	// calculate maximum size
	for (int i = 0; i < len; i++)
	{
		XojPageView* v = this->view->viewPages[i];

		int rId = 0;
		if (dualPage && i % 2 == 1)
		{
			rId = 1;
		}

		if (size[rId] < v->getDisplayWidth())
		{
			size[rId] = v->getDisplayWidth();
		}
	}


	int marginLeft = 0;
	int marginTop = 0;

	y += XOURNAL_PADDING;

	int width = XOURNAL_PADDING + size[0];
	if (dualPage)
	{
		width += XOURNAL_PADDING_BETWEEN + size[1] + XOURNAL_PADDING;
	}
	else
	{
		width += XOURNAL_PADDING;
	}

	Rectangle visRect = getVisibleRect();

	marginLeft = MAX(XOURNAL_PADDING, (visRect.width - width) / 2.f);

	if (horizontalSpace)
	{
		//A quarter of the document is always visible in window
		marginLeft = MAX(marginLeft, visRect.width * 0.75);
	}

	int verticalSpaceBetweenSlides = 0;

	if (len > 0 && verticalSpace)
	{
		marginTop = MAX(marginTop, visRect.height * 0.75);
	}

	for (int i = 0; i < len; i++)
	{
		XojPageView* v = this->view->viewPages[i];

		int height = 0;

		if (dualPage)
		{
			/*
			 * Align the left page right and the right page left, like this
			 * (first page at right)
			 *
			 *       [===]
			 *  [==] [=]
			 * [===] [===]
			 */
			if (i == 0)
			{
				int x = XOURNAL_PADDING + size[0] + XOURNAL_PADDING_BETWEEN;

				v->layout.setX(x);
				v->layout.setY(y);
				v->layout.setMarginLeft(marginLeft);
				v->layout.setMarginTop(marginTop);

				height = v->getDisplayHeight();
			}
			else
			{
				XojPageView* v2 = NULL;
				height = v->getDisplayHeight();

				if (i + 1 < len)
				{
					i++;

					v2 = this->view->viewPages[i];
					if (height < v2->getDisplayHeight())
					{
						height = v2->getDisplayHeight();
					}
				}

				// left page, align right
				int x1 = XOURNAL_PADDING + (size[0] - v->getDisplayWidth());
				v->layout.setX(x1);
				v->layout.setY(y);
				v->layout.setMarginLeft(marginLeft);
				v->layout.setMarginTop(marginTop);

				// if right page available, align left
				if (v2 != NULL)
				{
					int x2 = XOURNAL_PADDING + size[0] + XOURNAL_PADDING_BETWEEN;

					v2->layout.setX(x2);
					v2->layout.setY(y);
					v2->layout.setMarginLeft(marginLeft);
					v2->layout.setMarginTop(marginTop);
				}
			}
		}
		else
		{
			/*
			 * Center vertically, like this
			 *
			 *  [=]
			 * [===]
			 * [===]
			 *  [=]
			 */
			int x = (size[0] - v->getDisplayWidth()) / 2 + XOURNAL_PADDING;

			v->layout.setX(x);
			v->layout.setY(y);
			v->layout.setMarginLeft(marginLeft);
			v->layout.setMarginTop(marginTop);

			height = v->getDisplayHeight();
		}

		y += height;

		y += XOURNAL_PADDING_BETWEEN + verticalSpaceBetweenSlides;
	}

	int height = 2 * marginTop + XOURNAL_PADDING + y + XOURNAL_PADDING;

	width += 2 * marginLeft;

	this->setLayoutSize(width, height);
	this->view->pagePosition->update(this->view->viewPages, len, height);
}

void Layout::setLayoutSize(int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	this->layoutHeight = height;
	this->layoutWidth = width;

	gtk_widget_queue_resize(view->getWidget());
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

	gtk_adjustment_set_value(adjHorizontal, gtk_adjustment_get_value(adjHorizontal) + x);
	gtk_adjustment_set_value(adjVertical, gtk_adjustment_get_value(adjVertical) + y);
}

void Layout::scrollAbs(int x, int y)
{
	XOJ_CHECK_TYPE(Layout);

	gtk_adjustment_set_value(adjHorizontal, x);
	gtk_adjustment_set_value(adjVertical, y);
}


void Layout::ensureRectIsVisible(int x, int y, int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	gtk_adjustment_clamp_page(adjHorizontal, x - 5, x + width + 10);
	gtk_adjustment_clamp_page(adjVertical, y - 5, y + height + 10);
}



