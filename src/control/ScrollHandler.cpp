#include "Control.h"
#include "ScrollHandler.h"

#include "gui/Layout.h"
#include "gui/widgets/SpinPageAdapter.h"
#include "gui/XournalView.h"

ScrollHandler::ScrollHandler(Control* control)
{
	XOJ_INIT_TYPE(ScrollHandler);

	this->control = control;
}

ScrollHandler::~ScrollHandler()
{
	XOJ_RELEASE_TYPE(ScrollHandler);
}

void ScrollHandler::goToPreviousPage()
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow())
	{
		if (this->control->getSettings()->isPresentationMode())
		{
			PageView* view = this->control->getWindow()->getXournal()->getViewFor(this->control->getWindow()->getXournal()->getCurrentPage() - 1);
			if (view)
			{
				double dHeight = view->getDisplayHeight();
				double disHeight = 0;
				double top = (dHeight - disHeight)/2.0 + 7.5;

				// the magic 7.5 is from XOURNAL_PADDING_BETWEEN/2
				scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() - 1, top);
			}
		}
		else
		{
			scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() - 1);
		}
	}
}

void ScrollHandler::goToNextPage()
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow())
	{
		if (this->control->getSettings()->isPresentationMode())
		{
			PageView* view = this->control->getWindow()->getXournal()->getViewFor(this->control->getWindow()->getXournal()->getCurrentPage() + 1);
			if (view)
			{
				double dHeight = view->getDisplayHeight();
				double disHeight = this->control->getWindow()->getLayout()->getLayoutHeight();

				// this gets reversed when we are going down if the page is smaller than the display height
				double top = (-dHeight + disHeight)/2.0 - 7.5;

				// the magic 7.5 is from XOURNAL_PADDING_BETWEEN/2
				scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() + 1, top);
			}
		}
		else
		{
			scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() + 1);
		}
	}
}

void ScrollHandler::goToLastPage()
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow())
	{
		scrollToPage(this->control->getDocument()->getPageCount() - 1);
	}
}

void ScrollHandler::goToFirstPage()
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow())
	{
		scrollToPage(0);
	}
}

void ScrollHandler::scrollToPage(PageRef page, double top)
{
	XOJ_CHECK_TYPE(ScrollHandler);

	Document* doc = this->control->getDocument();

	doc->lock();
	int p = doc->indexOf(page);
	doc->unlock();

	if (p != -1)
	{
		scrollToPage(p, top);
	}
}

void ScrollHandler::scrollToPage(size_t page, double top)
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow())
	{
		this->control->getWindow()->getXournal()->scrollTo(page, top);
	}
}

void ScrollHandler::scrollToSpinPange()
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (!this->control->getWindow())
	{
		return;
	}
	SpinPageAdapter* spinPageNo = this->control->getWindow()->getSpinPageNo();
	int page = spinPageNo->getPage();
	if (page == 0)
	{
		return;
	}
	scrollToPage(page - 1);
}

void ScrollHandler::scrollToAnnotatedPage(bool next)
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (!this->control->getWindow())
	{
		return;
	}

	int step = next ? 1 : -1;

	Document* doc = this->control->getDocument();

	for (size_t i = this->control->getCurrentPageNo() + step; i != size_t_npos && i < doc->getPageCount();
		 i = ((i == 0 && step == -1) ? size_t_npos : i + step))
	{
		if (doc->getPage(i)->isAnnotated())
		{
			scrollToPage(i);
			break;
		}
	}
}

bool ScrollHandler::isPageVisible(size_t page, int* visibleHeight)
{
	XOJ_CHECK_TYPE(ScrollHandler);

	if (!this->control->getWindow())
	{
		if (visibleHeight)
		{
			*visibleHeight = 0;
		}
		return false;
	}

	return this->control->getWindow()->getXournal()->isPageVisible(page, visibleHeight);
}

void ScrollHandler::pageChanged(size_t page)
{
	XOJ_CHECK_TYPE(ScrollHandler);

	scrollToSpinPange();
}
