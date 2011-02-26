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
#include "../../gui/sidebar/SidebarPreview.h"


class PreviewJob : public Job {
public:
	PreviewJob(SidebarPreview * sidebar);
	virtual ~PreviewJob();

public:
	virtual void * getSource();

	virtual void run();

	virtual JobType getType();

private:
	SidebarPreview * sidebarPreview;
};

#endif /* __PREVIEWJOB_H__ */
