#include "BlockingJob.h"

#include <glib.h>     // for g_idle_add, GSourceFunc
#include <gtk/gtk.h>  // for gtk_widget_grab_focus

#include "control/Control.h"   // for Control
#include "control/jobs/Job.h"  // for JOB_TYPE_BLOCKING, JobType
#include "gui/MainWindow.h"    // for MainWindow
#include "gui/XournalView.h"   // for XournalView
#include "util/Util.h"         // for execInUiThread

BlockingJob::BlockingJob(Control* control, const std::string& name): control(control) { control->block(name); }

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
