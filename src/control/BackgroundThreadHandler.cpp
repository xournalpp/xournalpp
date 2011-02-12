#include "BackgroundThreadHandler.h"

#include "Control.h"
#include <unistd.h>

BackgroundThreadHandler::BackgroundThreadHandler(Control * control) {
	this->control = control;
	this->runnable = NULL;
	this->thread = thread;
}

BackgroundThreadHandler::~BackgroundThreadHandler() {
	this->control = NULL;

	// The Thread will be deleted on stop
	this->runnable = NULL;
	this->thread = NULL;
}

int BackgroundThreadHandler::threadCallback(BackgroundThreadHandler * th) {
	th->cancel = false;
	th->runnable->run(&th->cancel);
	delete th->runnable;
	th->runnable = NULL;
	th->thread = NULL;

	g_idle_add((GSourceFunc) finished, th);
	return 0;
}

bool BackgroundThreadHandler::finished(BackgroundThreadHandler * th) {
	th->control->getWindow()->setControlTmpDisabled(false);
	th->control->getCursor()->setCursorBusy(false);
	th->control->setSidebarTmpDisabled(false);

	// Do not call again
	return false;
}

void BackgroundThreadHandler::run(Runnable * runnable) {
	if (runnable == NULL) {
		g_critical("BackgroundThreadHandler::run(runnable == NULL)!");
		return;
	}

	if (this->runnable) {
		g_critical("There is already a background operation in progress");
		delete runnable;
		return;
	}

	this->runnable = runnable;

	// Disable all gui Control, to get full control over the application
	control->getWindow()->setControlTmpDisabled(true);
	control->getCursor()->setCursorBusy(true);
	control->setSidebarTmpDisabled(true);

	this->thread = g_thread_create((GThreadFunc) threadCallback, this, false, NULL);

}

bool BackgroundThreadHandler::isRunning() {
	return this->runnable != NULL;
}

void BackgroundThreadHandler::stop() {
	this->cancel = true;
	while(this->runnable) {
		usleep(1000);
	}
}

