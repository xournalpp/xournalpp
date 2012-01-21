#include "XournalScheduler.h"
#include "PreviewJob.h"
#include "RenderJob.h"

XournalScheduler::XournalScheduler() {
	XOJ_INIT_TYPE(XournalScheduler);
}

XournalScheduler::~XournalScheduler() {
	XOJ_RELEASE_TYPE(XournalScheduler);
}

void XournalScheduler::removeSidebar(SidebarPreviewPage * preview) {
	XOJ_CHECK_TYPE(XournalScheduler);

	removeSource(preview, JOB_TYPE_PREVIEW, JOB_PRIORITY_HIGH);
}

void XournalScheduler::removePage(PageView * view) {
	XOJ_CHECK_TYPE(XournalScheduler);

	removeSource(view, JOB_TYPE_RENDER, JOB_PRIORITY_URGENT);
}

void XournalScheduler::removeAllJobs() {
	XOJ_CHECK_TYPE(XournalScheduler);

	g_mutex_lock(this->jobQueueMutex);

	for(int priority = JOB_PRIORITY_URGENT; priority < JOB_N_PRIORITIES; priority++) {
		int length = g_queue_get_length(this->jobQueue[priority]);
		for (int i = 0; i < length; i++) {
			Job * job = (Job *) g_queue_peek_nth(this->jobQueue[priority], i);

			JobType type = job->getType();
			if (type == JOB_TYPE_PREVIEW || type == JOB_TYPE_RENDER) {
				job->deleteJob();
				g_queue_remove(this->jobQueue[priority], job);
				job->unref();
			}
		}
	}

	g_mutex_unlock(this->jobQueueMutex);

}

void XournalScheduler::finishTask() {
	XOJ_CHECK_TYPE(XournalScheduler);

	g_mutex_lock(this->jobRunningMutex);
	g_mutex_unlock(this->jobRunningMutex);
}

void XournalScheduler::removeSource(void * source, JobType type, JobPriority priority) {
	XOJ_CHECK_TYPE(XournalScheduler);

	g_mutex_lock(this->jobQueueMutex);

	int length = g_queue_get_length(this->jobQueue[priority]);
	for (int i = 0; i < length; i++) {
		Job * job = (Job *) g_queue_peek_nth(this->jobQueue[priority], i);

		if (job->getType() == type) {
			if (job->getSource() == source) {
				job->deleteJob();
				g_queue_remove(this->jobQueue[priority], job);
				job->unref();
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
	XOJ_CHECK_TYPE(XournalScheduler);

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

void XournalScheduler::addRepaintSidebar(SidebarPreviewPage * preview) {
	XOJ_CHECK_TYPE(XournalScheduler);

	if (existsSource(preview, JOB_TYPE_PREVIEW, JOB_PRIORITY_HIGH)) {
		return;
	}

	PreviewJob * job = new PreviewJob(preview);
	addJob(job, JOB_PRIORITY_HIGH);
	job->unref();
}

void XournalScheduler::addRerenderPage(PageView * view) {
	XOJ_CHECK_TYPE(XournalScheduler);

	if (existsSource(view, JOB_TYPE_RENDER, JOB_PRIORITY_URGENT)) {
		return;
	}

	RenderJob * job = new RenderJob(view);
	addJob(job, JOB_PRIORITY_URGENT);
	job->unref();
}

