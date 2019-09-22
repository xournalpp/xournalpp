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
	void run();

protected:
	virtual void addFilterToDialog();
	virtual bool isUriValid(string& uri);

private:
	};
