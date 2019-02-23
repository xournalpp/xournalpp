#include "PageRef.h"

#include "BackgroundImage.h"
#include "XojPage.h"

PageRef::PageRef()
{
	XOJ_INIT_TYPE(PageRef);
}

PageRef::PageRef(const PageRef& ref)
{
	XOJ_INIT_TYPE(PageRef);
	this->page = ref.page;
	if (this->page)
	{
		this->page->reference();
	}
}

PageRef::PageRef(XojPage* page)
{
	XOJ_INIT_TYPE(PageRef);
	this->page = page;
	if (this->page)
	{
		this->page->reference();
	}
}

PageRef::~PageRef()
{
	XOJ_CHECK_TYPE(PageRef);

	if (this->page)
	{
		this->page->unreference();
		this->page = NULL;
	}

	XOJ_RELEASE_TYPE(PageRef);
}

void PageRef::operator=(const PageRef& ref)
{
	*this = ref.page;
}

void PageRef::operator=(XojPage* page)
{
	if (this->page)
	{
		this->page->unreference();
	}
	this->page = page;
	if (this->page)
	{
		this->page->reference();
	}
}

XojPage& PageRef::operator*()
{
	XOJ_CHECK_TYPE(PageRef);

	return *(this->page);
}

XojPage* PageRef::operator->()
{
	XOJ_CHECK_TYPE(PageRef);

	return this->page;
}

const XojPage& PageRef::operator*() const
{
	XOJ_CHECK_TYPE(PageRef);

	return *(this->page);
}

const XojPage* PageRef::operator->() const
{
	XOJ_CHECK_TYPE(PageRef);

	return this->page;
}

PageRef PageRef::clone()
{
	if (this->page == NULL)
	{
		return PageRef(NULL);
	}
	else
	{
		return PageRef(this->page->clone());
	}
}

bool PageRef::isValid()
{
	XOJ_CHECK_TYPE(PageRef);

	return this->page != NULL;
}

PageRef::operator XojPage* ()
{
	XOJ_CHECK_TYPE(PageRef);

	return this->page;
}

bool PageRef::operator==(const PageRef& ref)
{
	XOJ_CHECK_TYPE(PageRef);

	return this->page == ref.page;
}
