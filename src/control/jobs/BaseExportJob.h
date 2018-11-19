/*
 * Xournal++
 *
 * Base class for Exports
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BlockingJob.h"

#include <StringUtils.h>
#include <XournalType.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class Control;

class BaseExportJob : public BlockingJob
{
public:
	BaseExportJob(Control* control, string name);

protected:
	virtual ~BaseExportJob();

public:
	void run(bool noThreads);
	virtual void afterRun();

public:
	bool showFilechooser();

private:
	XOJ_TYPE_ATTRIB;

	path filename;

	string errorMsg;
};
