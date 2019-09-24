/*
 * Xournal++
 *
 * Autosave job
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Job.h"

#include <XournalType.h>

class Control;

class AutosaveJob : public Job
{
public:
	AutosaveJob(Control* control);

protected:
	virtual ~AutosaveJob();

public:
	virtual void run();
	void afterRun();

	virtual JobType getType();

private:
	Control* control = nullptr;
	string error;
};
