/*
 * Xournal++
 *
 * A job which is done in the GTK main thread, but the application is blocked
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "Job.h"  // for Job, JobType

class Control;

class BlockingJob: public Job {
public:
    BlockingJob(Control* control, const std::string& name);

protected:
    ~BlockingJob() override;

public:
    void execute() override;

    JobType getType() override;

protected:
    static bool finished(Control* control);

private:
protected:
    Control* control;
};
