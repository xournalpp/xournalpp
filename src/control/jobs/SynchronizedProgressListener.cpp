#include "SynchronizedProgressListener.h"
#include <gtk/gtk.h>

SynchronizedProgressListener::SynchronizedProgressListener(ProgressListener * target) {
	this->target = target;
}

SynchronizedProgressListener::~SynchronizedProgressListener() {
	this->target = NULL;
}

void SynchronizedProgressListener::setMaximumState(int max) {
	gdk_threads_enter();
	this->target->setMaximumState(max);
	gdk_threads_leave();
}

void SynchronizedProgressListener::setCurrentState(int state) {
	gdk_threads_enter();
	this->target->setCurrentState(state);
	gdk_threads_leave();
}
