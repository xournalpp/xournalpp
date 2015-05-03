#include "SidebarPreviewPages.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "../base/SidebarToolbar.h"
#include "SidebarPreviewPageEntry.h"

SidebarPreviewPages::SidebarPreviewPages(Control* control) : SidebarPreviewBase(control)
{
	XOJ_INIT_TYPE(SidebarPreviewPages);
}

SidebarPreviewPages::~SidebarPreviewPages()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);
	XOJ_RELEASE_TYPE(SidebarPreviewPages);
}

const char* SidebarPreviewPages::getName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return _("Page Preview");
}

const char* SidebarPreviewPages::getIconName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return "sidebar-page-preview.svg";
}


void SidebarPreviewPages::updatePreviews()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	Document* doc = this->getControl()->getDocument();
	doc->lock();
	int len = doc->getPageCount();

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

	for (int i = 0; i < len; i++)
	{
		SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(i));
		this->previews.push_back(p);
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	layout();
	doc->unlock();
}


void SidebarPreviewPages::pageSizeChanged(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page < 0 || page >= this->previews.size())
	{
		return;
	}
	SidebarPreviewBaseEntry* p = this->previews[page];
	p->updateSize();
	p->repaint();

	layout();
}

void SidebarPreviewPages::pageChanged(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page < 0 || page >= this->previews.size())
	{
		return;
	}

	SidebarPreviewBaseEntry* p = this->previews[page];
	p->repaint();
}


void SidebarPreviewPages::pageDeleted(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	delete this->previews[page];
	this->previews.erase(this->previews.begin() + page);

	layout();
}

void SidebarPreviewPages::pageInserted(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	Document* doc = control->getDocument();
	doc->lock();

	SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(page));

	doc->unlock();

	this->previews.insert(this->previews.begin() + page, p);

	gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

	layout();
}

void SidebarPreviewPages::pageSelected(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (this->selectedEntry >= 0 && this->selectedEntry < this->previews.size())
	{
		this->previews[this->selectedEntry]->setSelected(false);
	}
	this->selectedEntry = page;

	if (this->selectedEntry >= 0 && this->selectedEntry < this->previews.size())
	{
		SidebarPreviewBaseEntry* p = this->previews[this->selectedEntry];
		p->setSelected(true);
		scrollToPreview(this);

		this->toolbar->setButtonEnabled(page != 0 && this->previews.size() != 0,
										page != this->previews.size() - 1 && this->previews.size() != 0,
										true, this->previews.size() > 1, this->control->getDocument()->getPage(page));
	}
}

