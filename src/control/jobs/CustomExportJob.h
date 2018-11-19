/*
 * Xournal++
 *
 * A customized export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseExportJob.h"

class CustomExportJob : public BaseExportJob
{
public:
	CustomExportJob(Control* control);

protected:
	virtual ~CustomExportJob();

public:
	void run(bool noThreads);

public:
	virtual bool showFilechooser();

protected:
	virtual void addFilterToDialog();
	virtual void addExtensionToFilePath();

private:
	XOJ_TYPE_ATTRIB;
};
