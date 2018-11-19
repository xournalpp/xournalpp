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

#include "BaseExportJob.h"

class PdfExportJob : public BaseExportJob
{
public:
	PdfExportJob(Control* control);

protected:
	virtual ~PdfExportJob();

public:
	void run(bool noThreads);

protected:
	virtual void addFilterToDialog();
	virtual void addExtensionToFilePath();
	virtual void prepareSavePath(path& path);

private:
	XOJ_TYPE_ATTRIB;
};
