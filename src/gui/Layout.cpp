#include "Layout.h"
#include "widgets/Scrollbar.h"
#include "XournalView.h"
#include "../control/Control.h"
#include "pageposition/PagePositionHandler.h"
#include "widgets/XournalWidget.h"

Layout::Layout(XournalView * view) {
	XOJ_INIT_TYPE(Layout);

	this->scrollVertical = new Scrollbar(false);
	this->scrollHorizontal = new Scrollbar(true);

	this->scrollVertical->addListener(this);
	this->scrollHorizontal->addListener(this);

	this->lastWidgetWidth = 0;

	this->layoutHeight = 0;
	this->layoutWidth = 0;

	this->view = view;
}

Layout::~Layout() {
	XOJ_RELEASE_TYPE(Layout);

	delete this->scrollHorizontal;
	this->scrollHorizontal = NULL;
	delete this->scrollVertical;
	this->scrollVertical = NULL;
	this->view = NULL;
}

void Layout::scrolled(Scrollbar * scrollbar) {
	updateRepaintWidget();
	checkSelectedPage();
}

/**
 * Check which page should be selected
 */
void Layout::checkSelectedPage() {

	GtkAllocation allocation = { 0 };
	gtk_widget_get_allocation(this->view->getWidget(), &allocation);

	int scrollY = this->scrollVertical->getValue();
	int scrollX = this->scrollHorizontal->getValue();

	Control * control = this->view->getControl();

	int viewHeight = allocation.height;
	int viewWidth = allocation.width;
	bool twoPages = control->getSettings()->isShowTwoPages();

	if (scrollY < 1) {
		if (twoPages && this->view->viewPagesLen > 1 && this->view->viewPages[1]->isSelected()) {
			// page 2 already selected
		} else {
			control->firePageSelected(0);
		}
		return;
	}

	int mostPageNr = 0;
	double mostPagePercent = 0;

	// next four pages are not marked as invisible,
	// because usually you scroll forward

	for (int page = 0; page < this->view->viewPagesLen; page++) {
		PageView * p = this->view->viewPages[page];
		int y = p->getY();
		int x = p->getX();

		int pageHeight = p->getDisplayHeight();
		int pageWidth = p->getDisplayWidth();

		if (y > scrollY + viewHeight) {
			p->setIsVisibel(false);
			for (; page < this->view->viewPagesLen; page++) {
				p = this->view->viewPages[page];
				p->setIsVisibel(false);
			}

			break;
		}
		if (y + pageHeight >= scrollY) {
			int startY = 0;
			int endY = pageHeight;
			if (y <= scrollY) {
				startY = scrollY - y;
			}
			if (y + pageHeight > scrollY + viewHeight) {
				endY = pageHeight - ((y + pageHeight) - (scrollY + viewHeight));
			}

			int startX = 0;
			int endX = pageWidth;

			if (x <= scrollX) {
				startX = scrollX - x;
			}
			if (x + pageWidth > scrollX + viewWidth) {
				endX = pageWidth - ((y + pageWidth) - (scrollX + viewWidth));
			}


			double percent = ((double) (endY - startY)) / ((double) pageHeight);
			percent *= ((double) (endX - startX)) / ((double) pageWidth);

			if (percent > mostPagePercent) {
				mostPagePercent = percent;
				mostPageNr = page;
			}

			p->setIsVisibel(true);
		} else {
			p->setIsVisibel(false);
		}
	}

	if (twoPages && mostPageNr < this->view->viewPagesLen - 1) {
		int y1 = this->view->viewPages[mostPageNr]->getY();
		int y2 = this->view->viewPages[mostPageNr + 1]->getY();

		if (y1 != y2 || !this->view->viewPages[mostPageNr + 1]->isSelected()) {
			// if the second page is selected DON'T select the first page.
			// Only select the first page if none is selected
			control->firePageSelected(mostPageNr);
		}
	} else {
		control->firePageSelected(mostPageNr);
	}
}

/**
 * Padding outside the pages, including shadow
 */
const int XOURNAL_PADDING = 10;

/**
 * Padding between the pages
 */
const int XOURNAL_PADDING_BETWEEN = 15;

void Layout::layoutPages() {
	XOJ_CHECK_TYPE(Layout);

	int y = 0;

	int len = this->view->viewPagesLen;

	Settings * settings = this->view->getControl()->getSettings();
	bool allowScrollOutsideThePage = settings->isAllowScrollOutsideThePage();
	bool dualPage = settings->isShowTwoPages();

	int size[2] = { 0, 0 };

	// we need at least 2 page for dual page view
	if (len < 2) {
		dualPage = false;
	}

	// calculate maximum size
	for (int i = 0; i < len; i++) {
		PageView * v = this->view->viewPages[i];

		int rId = 0;
		if (dualPage && i % 2 == 1) {
			rId = 1;
		}

		if (size[rId] < v->getDisplayWidth()) {
			size[rId] = v->getDisplayWidth();
		}
	}


	int marginLeft = 0;
	int marginRight = 0;
	int marginTop = 0;
	int marginBottom = 0;

	y += XOURNAL_PADDING;

	int width = XOURNAL_PADDING + size[0];
	if (dualPage) {
		width += XOURNAL_PADDING_BETWEEN + size[1] + XOURNAL_PADDING;
	} else {
		width += XOURNAL_PADDING;
	}

	GtkAllocation alloc;
	gtk_widget_get_allocation(this->view->getWidget(), &alloc);

	marginLeft = marginRight = (alloc.width - width) / 2;
	marginLeft = MAX(marginLeft, 10);
	marginRight = MAX(marginRight, 10);

	if (allowScrollOutsideThePage) {
		marginLeft += size[0] / 2;
		if (dualPage) {
			marginRight += size[1] / 2;
		} else {
			marginRight += size[0] / 2;
		}
		if (len > 0) {
			marginTop += this->view->viewPages[0]->getDisplayHeight() / 2;
			marginBottom += this->view->viewPages[len - 1]->getDisplayHeight() / 2;
		}
	}

	for (int i = 0; i < len; i++) {
		PageView * v = this->view->viewPages[i];

		int height = 0;

		if (dualPage) {
			/**
			 * Align the left page right and the right page left, like this
			 * (first page at right)
			 *
			 *       [===]
			 *  [==] [=]
			 * [===] [===]
			 */
			if (i == 0) {
				int x = XOURNAL_PADDING + size[0] + XOURNAL_PADDING_BETWEEN;

				v->layout.setX(x);
				v->layout.setY(y);
				v->layout.setMarginLeft(marginLeft);
				v->layout.setMarginTop(marginTop);

				height = v->getDisplayHeight();
			} else {
				PageView * v2 = NULL;
				height = v->getDisplayHeight();

				if (i + 1 < len) {
					i++;

					v2 = this->view->viewPages[i];
					if (height < v2->getDisplayHeight()) {
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
				if (v2 != NULL) {
					int x2 = XOURNAL_PADDING + size[0] + XOURNAL_PADDING_BETWEEN;

					v2->layout.setX(x2);
					v2->layout.setY(y);
					v2->layout.setMarginLeft(marginLeft);
					v2->layout.setMarginTop(marginTop);
				}
			}
		} else {
			/**
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

		y += XOURNAL_PADDING_BETWEEN;
	}

	int height = marginTop + XOURNAL_PADDING + y + XOURNAL_PADDING + marginBottom;

	this->setLayoutSize(marginLeft + width + marginRight, height);
	this->view->pagePosition->update(this->view->viewPages, len, height);

	this->updateRepaintWidget();
}

void Layout::updateRepaintWidget() {
	XOJ_CHECK_TYPE(Layout);

	GtkWidget * widget = this->view->getWidget();
	GtkXournal * xournal = GTK_XOURNAL(widget);

	xournal->x = this->scrollHorizontal->getValue();
	xournal->y = this->scrollVertical->getValue();

	gtk_widget_queue_draw(widget);
}

void Layout::setLayoutSize(int width, int height) {
	XOJ_CHECK_TYPE(Layout);

	this->layoutHeight = height;
	this->layoutWidth = width;

	GtkAllocation alloc;
	gtk_widget_get_allocation(this->view->getWidget(), &alloc);

	this->scrollHorizontal->setMax(MAX(alloc.width, this->layoutWidth));
	this->scrollHorizontal->setPageIncrement(alloc.width * 0.9);

	this->scrollVertical->setMax(MAX(alloc.height, this->layoutHeight));
	this->scrollVertical->setPageIncrement(alloc.height * 0.9);

	this->scrollHorizontal->setPageSize(alloc.width);
	this->scrollVertical->setPageSize(alloc.height);
}

GtkWidget * Layout::getScrollbarVertical() {
	XOJ_CHECK_TYPE(Layout);

	return this->scrollVertical->getWidget();
}

GtkWidget * Layout::getScrollbarHorizontal() {
	XOJ_CHECK_TYPE(Layout);

	return this->scrollHorizontal->getWidget();
}

void Layout::setSize(int widgetWidth, int widgetHeight) {
	XOJ_CHECK_TYPE(Layout);

	if (this->lastWidgetWidth != widgetWidth) {
		this->layoutPages();
		this->lastWidgetWidth = widgetWidth;
	} else {
		this->setLayoutSize(this->layoutWidth, this->layoutHeight);
	}

	this->view->getControl()->calcZoomFitSize();
}

void Layout::scrollRelativ(int x, int y) {
	XOJ_CHECK_TYPE(Layout);

	this->scrollHorizontal->scroll(x);
	this->scrollVertical->scroll(y);
}

double Layout::getVisiblePageTop(int page) {
	XOJ_CHECK_TYPE(Layout);

	if (page < 0) {
		return 0;
	}
	if (page > this->view->viewPagesLen) {
		return 0;
	}

	if (this->view->viewPagesLen == 0) {
		return 0;
	}

	double y = this->view->viewPages[page]->getY() + this->scrollVertical->getValue();

	return y / this->view->getZoom();
}

void Layout::ensureRectIsVisible(int x, int y, int width, int height) {
	XOJ_CHECK_TYPE(Layout);

	this->scrollHorizontal->ensureAreaIsVisible(x - 5, x + width + 10);
	this->scrollVertical->ensureAreaIsVisible(y - 5, y + height + 10);
}

bool Layout::scrollEvent(GdkEventScroll * event) {
	XOJ_CHECK_TYPE(Layout);

	Scrollbar * scroll;
	if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN) {
		scroll = this->scrollVertical;
	} else {
		scroll = this->scrollHorizontal;
	}

	if (scroll && gtk_widget_get_visible(scroll->getWidget())) {
		double delta = scroll->getWheelDelta(event->direction);
		scroll->scroll(delta);
		return true;
	}

	return false;
}

