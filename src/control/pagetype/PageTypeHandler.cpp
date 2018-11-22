#include "PageTypeHandler.h"

#include <i18n.h>


PageTypeHandler::PageTypeHandler()
{
	XOJ_INIT_TYPE(PageTypeHandler);

	addPageTypeInfo(_C("Plain"), "plain", "");
	addPageTypeInfo(_C("Lined"), "lined", "");
	addPageTypeInfo(_C("Ruled"), "ruled", "");
	addPageTypeInfo(_C("Graph"), "graph", "");
	addPageTypeInfo(_C("Dotted"), "dotted", "");

	// Special types
	addPageTypeInfo(_C("Copy current"), ":copy", "");
	addPageTypeInfo(_C("With PDF background"), ":pdf", "");
}

PageTypeHandler::~PageTypeHandler()
{
	XOJ_CHECK_TYPE(PageTypeHandler);

	for (PageTypeInfo* t : types)
	{
		delete t;
	}
	types.clear();

	XOJ_RELEASE_TYPE(PageTypeHandler);
}

void PageTypeHandler::addPageTypeInfo(string name, string format, string config)
{
	PageTypeInfo* pt = new PageTypeInfo();
	pt->name = name;
	pt->page.format = format;
	pt->page.config = config;

	this->types.push_back(pt);
}

vector<PageTypeInfo*>& PageTypeHandler::getPageTypes()
{
	XOJ_CHECK_TYPE(PageTypeHandler);

	return this->types;
}
