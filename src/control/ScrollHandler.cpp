#include "Control.h"
#include "ScrollHandler.h"

#include "gui/Layout.h"
#include "gui/widgets/SpinPageAdapter.h"
#include "gui/XournalView.h"

ScrollHandler::ScrollHandler(Control* control)
 : control(control)
{
}

ScrollHandler::~ScrollHandler()
{
}

void ScrollHandler::goToPreviousPage()
{
	if (this->control->getWindow())
	{
		scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() - 1);
	}
}

void ScrollHandler::goToNextPage()
{
	if (this->control->getWindow())
	{
		scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() + 1);
	}
}

void ScrollHandler::goToLastPage()
{
	if (this->control->getWindow())
	{
		scrollToPage(this->control->getDocument()->getPageCount() - 1);
	}
}

void ScrollHandler::goToFirstPage()
{
	if (this->control->getWindow())
	{
		scrollToPage(0);
	}
}

void ScrollHandler::scrollToPage(const PageRef& page, double top)
{
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
	MainWindow* win = this->control->getWindow();
	if (win == nullptr)
	{
		g_error("Windows is nullptr!");
		return;
	}

	win->getXournal()->scrollTo(page, top);
}

void ScrollHandler::scrollToSpinPage()
{
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
	if (!this->control->getWindow())
	{
		return;
	}

	int step = next ? 1 : -1;

	Document* doc = this->control->getDocument();

	for (size_t i = this->control->getCurrentPageNo() + step; i != npos && i < doc->getPageCount();
	     i = ((i == 0 && step == -1) ? npos : i + step))
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
	scrollToSpinPage();
}
