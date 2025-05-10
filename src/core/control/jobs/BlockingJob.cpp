#include "BlockingJob.h"

#include <glib.h>  // for g_idle_add

#include "control/Control.h"   // for Control
#include "control/jobs/Job.h"  // for JOB_TYPE_BLOCKING, JobType
#include "gui/MainWindow.h"    // for MainWindow
#include "gui/XournalView.h"   // for XournalView
#include "util/Util.h"         // for execInUiThread
#include "util/glib_casts.h"   // for wrap_for_once_v

BlockingJob::BlockingJob(Control* control, const std::string& name): control(control) { control->block(name); }

BlockingJob::~BlockingJob() { this->control = nullptr; }

void BlockingJob::execute() {
    this->run();

    Util::execInUiThread([control = this->control]() {
        control->unblock();
        control->getWindow()->getXournal()->requestFocus();
    });
}

auto BlockingJob::getType() -> JobType { return JOB_TYPE_BLOCKING; }
