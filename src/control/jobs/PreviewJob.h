/*
 * Xournal++
 *
 * A job which handles preview repaint
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PREVIEWJOB_H__
#define __PREVIEWJOB_H__

#include "Job.h"
#include <XournalType.h>

class SidebarPreviewPage;

class PreviewJob : public Job {
public:
	PreviewJob(SidebarPreviewPage * sidebar);

protected:
	virtual ~PreviewJob();

public:
	virtual void * getSource();

	virtual void run();

	virtual JobType getType();

private:
	XOJ_TYPE_ATTRIB;

	SidebarPreviewPage * sidebarPreview;
};

#endif /* __PREVIEWJOB_H__ */
