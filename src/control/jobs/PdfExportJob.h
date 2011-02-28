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

#include "BlockingJob.h"

class Control;
class PdfExportJob: public BlockingJob {
public:
	PdfExportJob(Control * control);
	virtual ~PdfExportJob();

public:
	void run();

};

#endif /* __PDFEXPORTJOB_H__ */
