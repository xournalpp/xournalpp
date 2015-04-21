/*
 * Xournal++
 *
 * A job to export PDF
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

#include "BlockingJob.h"
#include <StringUtils.h>
#include <XournalType.h>

class Control;

class PdfExportJob : public BlockingJob
{
public:
	PdfExportJob(Control* control);

protected:
	virtual ~PdfExportJob();

public:
	void run();
	virtual void afterRun();

public:
	bool showFilechooser();

private:
	XOJ_TYPE_ATTRIB;

	path filename;

	string errorMsg;
};
