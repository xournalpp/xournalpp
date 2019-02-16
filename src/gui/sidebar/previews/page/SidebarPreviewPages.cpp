#include "SidebarPreviewPages.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "gui/sidebar/previews/base/SidebarToolbar.h"
#include "SidebarPreviewPageEntry.h"
#include "undo/CopyUndoAction.h"
#include "undo/SwapUndoAction.h"

#include <i18n.h>

SidebarPreviewPages::SidebarPreviewPages(Control* control, GladeGui* gui, SidebarToolbar* toolbar)
 : SidebarPreviewBase(control, gui, toolbar)
{
	XOJ_INIT_TYPE(SidebarPreviewPages);
}

SidebarPreviewPages::~SidebarPreviewPages()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);
	XOJ_RELEASE_TYPE(SidebarPreviewPages);
}

string SidebarPreviewPages::getName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return _("Page Preview");
}

string SidebarPreviewPages::getIconName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return "sidebar-page-preview";
}

/**
 * Called when an action is performed
 */
void SidebarPreviewPages::actionPerformed(SidebarActions action)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	switch (action)
	{
	case SIDEBAR_ACTION_MOVE_UP:
	{
		Document* doc = control->getDocument();
		PageRef swappedPage = control->getCurrentPage();
		if (!swappedPage.isValid())
		{
			return;
		}

		doc->lock();
		size_t page = doc->indexOf(swappedPage);
		PageRef otherPage = doc->getPage(page - 1);
		if (page != size_t_npos)
		{
			doc->deletePage(page);
			doc->insertPage(swappedPage, page - 1);
		}
		doc->unlock();

		UndoRedoHandler* undo = control->getUndoRedoHandler();
		undo->addUndoAction(new SwapUndoAction(page - 1, true, swappedPage, otherPage));

		control->firePageDeleted(page);
		control->firePageInserted(page - 1);
		control->firePageSelected(page - 1);

		control->getScrollHandler()->scrollToPage(page - 1);
		break;
	}
	case SIDEBAR_ACTION_MODE_DOWN:
	{
		Document* doc = control->getDocument();
		PageRef swappedPage = control->getCurrentPage();
		if (!swappedPage.isValid())
		{
			return;
		}

		doc->lock();
		size_t page = doc->indexOf(swappedPage);
		PageRef otherPage = doc->getPage(page + 1);
		if (page != size_t_npos)
		{
			doc->deletePage(page);
			doc->insertPage(swappedPage, page + 1);
		}
		doc->unlock();

		UndoRedoHandler* undo = control->getUndoRedoHandler();
		undo->addUndoAction(new SwapUndoAction(page, false, swappedPage, otherPage));

		control->firePageDeleted(page);
		control->firePageInserted(page + 1);
		control->firePageSelected(page + 1);

		control->getScrollHandler()->scrollToPage(page + 1);
		break;
	}
	case SIDEBAR_ACTION_COPY:
	{
		Document* doc = control->getDocument();
		PageRef currentPage = control->getCurrentPage();
		if (!currentPage.isValid())
		{
			return;
		}

		doc->lock();
		size_t page = doc->indexOf(currentPage);

		PageRef newPage = currentPage.clone();
		doc->insertPage(newPage, page + 1);

		doc->unlock();

		UndoRedoHandler* undo = control->getUndoRedoHandler();
		undo->addUndoAction(new CopyUndoAction(newPage, page + 1));

		control->firePageInserted(page + 1);
		control->firePageSelected(page + 1);

		control->getScrollHandler()->scrollToPage(page + 1);
		break;
	}
	case SIDEBAR_ACTION_DELETE:
		control->deletePage();
		break;
	default:
		break;
	}
}

void SidebarPreviewPages::updatePreviews()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	Document* doc = this->getControl()->getDocument();
	doc->lock();
	size_t len = doc->getPageCount();

	if (this->previews.size() == len)
	{
		doc->unlock();
		return;
	}

	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		delete p;
	}
	this->previews.clear();

	for (size_t i = 0; i < len; i++)
	{
		SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(i));
		this->previews.push_back(p);
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	layout();
	doc->unlock();
}

void SidebarPreviewPages::pageSizeChanged(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page == size_t_npos || page >= this->previews.size())
	{
		return;
	}
	SidebarPreviewBaseEntry* p = this->previews[page];
	p->updateSize();
	p->repaint();

	layout();
}

void SidebarPreviewPages::pageChanged(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page == size_t_npos || page >= this->previews.size())
	{
		return;
	}

	SidebarPreviewBaseEntry* p = this->previews[page];
	p->repaint();
}

void SidebarPreviewPages::pageDeleted(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page >= previews.size())
	{
		return;
	}

	delete previews[page];
	previews.erase(previews.begin() + page);

	// Unselect page, to prevent double selection displaying
	unselectPage();

	layout();
}

void SidebarPreviewPages::pageInserted(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	Document* doc = control->getDocument();
	doc->lock();

	SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(page));

	doc->unlock();

	this->previews.insert(this->previews.begin() + page, p);

	gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

	// Unselect page, to prevent double selection displaying
	unselectPage();

	layout();
}

/**
 * Unselect the last selected page, if any
 */
void SidebarPreviewPages::unselectPage()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		p->setSelected(false);
	}
}

void SidebarPreviewPages::pageSelected(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
	{
		this->previews[this->selectedEntry]->setSelected(false);
	}
	this->selectedEntry = page;

	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
	{
		SidebarPreviewBaseEntry* p = this->previews[this->selectedEntry];
		p->setSelected(true);
		scrollToPreview(this);

		int actions = 0;
		if (page != 0 && this->previews.size() != 0)
		{
			actions |= SIDEBAR_ACTION_MOVE_UP;
		}

		if (page != this->previews.size() - 1 && this->previews.size() != 0)
		{
			actions |= SIDEBAR_ACTION_MODE_DOWN;
		}

		if (this->previews.size() != 0)
		{
			actions |= SIDEBAR_ACTION_COPY;
		}

		if (this->previews.size() > 1)
		{
			actions |= SIDEBAR_ACTION_DELETE;
		}

		this->toolbar->setHidden(false);
		this->toolbar->setButtonEnabled((SidebarActions)actions);
	}
}

