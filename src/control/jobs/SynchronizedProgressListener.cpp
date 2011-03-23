#include "SynchronizedProgressListener.h"
#include <gtk/gtk.h>
// TODO: AA: type check

SynchronizedProgressListener::SynchronizedProgressListener(ProgressListener * target) {
	this->target = target;

	this->maxIdleId = 0;
	this->currentIdleId = 0;

	this->max = 0;
	this->current = 0;
}

SynchronizedProgressListener::~SynchronizedProgressListener() {
	this->target = NULL;

	if (this->maxIdleId) {
		g_source_remove(this->maxIdleId);
		this->maxIdleId = 0;
	}
	if (this->currentIdleId) {
		g_source_remove(this->currentIdleId);
		this->currentIdleId = 0;
	}
}

bool SynchronizedProgressListener::setMaxCallback(SynchronizedProgressListener * listener) {
	gdk_threads_enter();
	listener->target->setMaximumState(listener->max);
	gdk_threads_leave();

	listener->maxIdleId = 0;
	return false; // do not call again
}

bool SynchronizedProgressListener::setCurrentCallback(SynchronizedProgressListener * listener) {
	gdk_threads_enter();
	listener->target->setCurrentState(listener->current);
	gdk_threads_leave();

	listener->currentIdleId = 0;
	return false; // do not call again
}

void SynchronizedProgressListener::setMaximumState(int max) {
	this->max = max;
	if(this->maxIdleId) {
		return;
	}
	this->maxIdleId = g_idle_add((GSourceFunc) setMaxCallback, this);
}

void SynchronizedProgressListener::setCurrentState(int state) {
	this->current = state;
	if(this->currentIdleId) {
		return;
	}
	this->currentIdleId = g_idle_add((GSourceFunc) setCurrentCallback, this);
}
