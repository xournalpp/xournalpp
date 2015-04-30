/*
 * Xournal++
 *
 * A job which handles preview repaint
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Job.h"

#include <XournalType.h>

class SidebarPreviewPage;

class PreviewJob : public Job
{
public:
	PreviewJob(SidebarPreviewPage* sidebar);

protected:
	virtual ~PreviewJob();

public:
	virtual void* getSource();

	virtual void run();

	virtual JobType getType();

private:
	XOJ_TYPE_ATTRIB;

	SidebarPreviewPage* sidebarPreview;
};
