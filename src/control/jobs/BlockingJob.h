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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "Job.h"
#include "XournalType.h"

class Control;

class BlockingJob: public Job {
public:
    BlockingJob(Control* control, const string& name);

protected:
    virtual ~BlockingJob();

public:
    void execute();

    virtual JobType getType();

protected:
    static bool finished(Control* control);

private:
protected:
    Control* control;
};
