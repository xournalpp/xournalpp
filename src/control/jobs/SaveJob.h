/*
 * Xournal++
 *
 * A job which saves a Document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __SAVEJOB_H__
#define __SAVEJOB_H__

#include "BlockingJob.h"
#include <StringUtils.h>
#include <XournalType.h>

class SaveJob : public BlockingJob
{
public:
	SaveJob(Control* control);

protected:
	virtual ~SaveJob();

public:
	virtual void run();

	bool save();

private:
	void updatePreview();
	virtual void afterRun();

private:
	XOJ_TYPE_ATTRIB;

	string lastError;

};

#endif /* __SAVEJOB_H__ */
