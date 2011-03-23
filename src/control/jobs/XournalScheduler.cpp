#include "XournalScheduler.h"
#include "PreviewJob.h"
#include "RenderJob.h"
// TODO: AA: type check

XournalScheduler::XournalScheduler() {
}

XournalScheduler::~XournalScheduler() {
}

void XournalScheduler::removeSidebar(SidebarPreview * preview) {
	removeSource(preview, JOB_TYPE_PREVIEW, JOB_PRIORITY_HIGH);
}

void XournalScheduler::removePage(PageView * view) {
	removeSource(view, JOB_TYPE_RENDER, JOB_PRIORITY_URGENT);
}

void XournalScheduler::finishTask() {
	g_mutex_lock(this->jobRunningMutex);
	g_mutex_unlock(this->jobRunningMutex);
}

void XournalScheduler::removeSource(void * source, JobType type, JobPriority priority) {
	g_mutex_lock(this->jobQueueMutex);

	int length = g_queue_get_length(this->jobQueue[priority]);
	for (int i = 0; i < length; i++) {
		Job * job = (Job *) g_queue_peek_nth(this->jobQueue[priority], i);

		CHECK_MEMORY(job);

		if (job->getType() == type) {
			if (job->getSource() == source) {
				g_queue_remove(this->jobQueue[priority], job);
				job->free();
				break;
			}
		}
	}

	// wait until the last job is done
	// we can be sure we don't access "source"
	finishTask();

	g_mutex_unlock(this->jobQueueMutex);
}

bool XournalScheduler::existsSource(void * source, JobType type, JobPriority priority) {
	bool exists = false;
	g_mutex_lock(this->jobQueueMutex);

	int length = g_queue_get_length(this->jobQueue[priority]);
	for (int i = 0; i < length; i++) {
		Job * job = (Job *) g_queue_peek_nth(this->jobQueue[priority], i);

		if (job->getType() == type) {
			if (job->getSource() == source) {
				exists = true;
				break;
			}
		}
	}

	g_mutex_unlock(this->jobQueueMutex);

	return exists;
}

void XournalScheduler::addRepaintSidebar(SidebarPreview * preview) {
	if (existsSource(preview, JOB_TYPE_PREVIEW, JOB_PRIORITY_HIGH)) {
		return;
	}

	addJob(new PreviewJob(preview), JOB_PRIORITY_HIGH);
}

void XournalScheduler::addRepaintPage(PageView * view) {
	if (existsSource(view, JOB_TYPE_RENDER, JOB_PRIORITY_URGENT)) {
		return;
	}

	addJob(new RenderJob(view), JOB_PRIORITY_URGENT);
}

