#include "DocumentListener.h"
#include "DocumentHandler.h"

DocumentListener::DocumentListener() {
	this->handler = NULL;
}

DocumentListener::~DocumentListener() {
	unregisterListener();
}

void DocumentListener::registerListener(DocumentHandler * handler) {
	this->handler = handler;
	handler->addListener(this);
}

void DocumentListener::unregisterListener() {
	if (this->handler) {
		this->handler->removeListener(this);
	}
}

