#include "BlockingJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"
#include "gui/XournalView.h"

BlockingJob::BlockingJob(Control* control, const string& name): control(control) { control->block(name); }

BlockingJob::~BlockingJob() { this->control = nullptr; }

void BlockingJob::execute() {
    this->run();

    g_idle_add(reinterpret_cast<GSourceFunc>(finished), this->control);
}

auto BlockingJob::finished(Control* control) -> bool {
    // "this" is not needed, "control" is in
    // the closure, therefore no sync needed
    Util::execInUiThread([=]() {
        control->unblock();
        XournalView* xournal = control->getWindow()->getXournal();
        gtk_widget_grab_focus(xournal->getWidget());
    });

    // do not call again
    return false;
}

auto BlockingJob::getType() -> JobType { return JOB_TYPE_BLOCKING; }
