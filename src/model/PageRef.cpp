#include "PageRef.h"

#include "BackgroundImage.h"
#include "XojPage.h"

PageRef::PageRef()
{
}

PageRef::PageRef(const PageRef& ref)
{
	this->page = ref.page;
	if (this->page)
	{
		this->page->reference();
	}
}

PageRef::PageRef(XojPage* page)
{
	this->page = page;
	if (this->page)
	{
		this->page->reference();
	}
}

PageRef::~PageRef()
{
	if (this->page)
	{
		this->page->unreference();
		this->page = nullptr;
	}
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
	return *(this->page);
}

XojPage* PageRef::operator->()
{
	return this->page;
}

const XojPage& PageRef::operator*() const
{
	return *(this->page);
}

const XojPage* PageRef::operator->() const
{
	return this->page;
}

PageRef PageRef::clone()
{
	if (this->page == nullptr)
	{
		return PageRef(nullptr);
	}
	else
	{
		return PageRef(this->page->clone());
	}
}

bool PageRef::isValid()
{
	return this->page != nullptr;
}

PageRef::operator XojPage* ()
{
	return this->page;
}

bool PageRef::operator==(const PageRef& ref)
{
	return this->page == ref.page;
}
