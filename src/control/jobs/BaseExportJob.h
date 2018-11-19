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
	virtual void afterRun();

public:
	virtual bool showFilechooser();

protected:
	void initDialog();
	virtual void addFilterToDialog() = 0;
	GtkFileFilter* addFileFilterToDialog(string name, string pattern);
	virtual void addExtensionToFilePath() = 0;
	virtual void prepareSavePath(path& path);

private:
	XOJ_TYPE_ATTRIB;

protected:
	GtkWidget* dialog;

	path filename;

	string errorMsg;
};
