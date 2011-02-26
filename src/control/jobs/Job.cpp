#include "Job.h"
#include <stdio.h>

Job::Job() {
}

Job::~Job() {
}

void Job::execute() {
	this->run();
}

void * Job::getSource() {
	return NULL;
}
