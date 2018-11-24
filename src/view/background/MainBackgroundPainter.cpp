#include "MainBackgroundPainter.h"

#include "BaseBackgroundPainter.h"
#include "DottedBackgroundPainter.h"
#include "GraphBackgroundPainter.h"
#include "LineBackgroundPainter.h"

using std::pair;

MainBackgroundPainter::MainBackgroundPainter()
{
	XOJ_INIT_TYPE(MainBackgroundPainter);

	defaultPainter = new BaseBackgroundPainter();

	painter.insert(pair<string, BaseBackgroundPainter*>("lined", new LineBackgroundPainter(false)));
	painter.insert(pair<string, BaseBackgroundPainter*>("ruled", new LineBackgroundPainter(true)));
	painter.insert(pair<string, BaseBackgroundPainter*>("graph", new GraphBackgroundPainter()));
	painter.insert(pair<string, BaseBackgroundPainter*>("dotted", new DottedBackgroundPainter()));
}

MainBackgroundPainter::~MainBackgroundPainter()
{
	XOJ_CHECK_TYPE(MainBackgroundPainter);

	for (auto& e : painter)
	{
		delete e.second;
	}
	painter.clear();

	delete defaultPainter;
	defaultPainter = NULL;

	XOJ_RELEASE_TYPE(MainBackgroundPainter);
}

void MainBackgroundPainter::paint(PageType pt, cairo_t* cr, PageRef page)
{
	XOJ_CHECK_TYPE(MainBackgroundPainter);

	auto it = this->painter.find(pt.format);

	BaseBackgroundPainter* painter = defaultPainter;
	if (it != this->painter.end())
	{
		painter = it->second;
	}

	map<string, string> config;
	// TODO Parse config

	painter->paint(cr, page, config);
}
