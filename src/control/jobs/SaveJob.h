/*
 * Xournal++
 *
 * A job which saves a Document
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
