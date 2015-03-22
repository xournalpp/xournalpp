/*
 * Xournal++
 *
 * A job to export PDF
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFEXPORTJOB_H__
#define __PDFEXPORTJOB_H__

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

#endif /* __PDFEXPORTJOB_H__ */
