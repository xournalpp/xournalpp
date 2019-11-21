#include "PageRef.h"

#include "BackgroundImage.h"
#include "XojPage.h"

PageRef::PageRef() = default;

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

auto PageRef::operator*() -> XojPage&
{
	return *(this->page);
}

auto PageRef::operator-> () -> XojPage*
{
	return this->page;
}

auto PageRef::operator*() const -> const XojPage&
{
	return *(this->page);
}

auto PageRef::operator-> () const -> const XojPage*
{
	return this->page;
}

auto PageRef::clone() -> PageRef
{
	if (this->page == nullptr)
	{
		return PageRef(nullptr);
	}


	    return PageRef(this->page->clone());
}

auto PageRef::isValid() -> bool
{
	return this->page != nullptr;
}

PageRef::operator XojPage* ()
{
	return this->page;
}

auto PageRef::operator==(const PageRef& ref) -> bool
{
	return this->page == ref.page;
}
