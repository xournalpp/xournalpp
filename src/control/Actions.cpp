#include "Actions.h"

ActionEnabledListener::ActionEnabledListener() {
	this->handler = NULL;
}

ActionEnabledListener::~ActionEnabledListener() {
	unregisterListener();
}

void ActionEnabledListener::registerListener(ActionHandler * handler) {
	if (this->handler == NULL) {
		this->handler = handler;
		this->handler->addListener(this);
	}
}

void ActionEnabledListener::unregisterListener() {
	if (this->handler) {
		this->handler->removeListener(this);
		this->handler = NULL;
	}
}

ActionSelectionListener::ActionSelectionListener() {
	this->handler = NULL;
}

ActionSelectionListener::~ActionSelectionListener() {
	unregisterListener();
}

void ActionSelectionListener::registerListener(ActionHandler * handler) {
	if (this->handler == NULL) {
		this->handler = handler;
		handler->addListener(this);
	}
}

void ActionSelectionListener::unregisterListener() {
	if (this->handler != NULL) {
		handler->removeListener(this);
		this->handler = handler;
	}
}


ActionHandler::ActionHandler() {
	this->enabledListener = NULL;
	this->selectionListener = NULL;
}

ActionHandler::~ActionHandler() {
	g_list_free(this->enabledListener);
	g_list_free(this->selectionListener);
}

void ActionHandler::fireEnableAction(ActionType action, bool enabled) {
	for(GList * l = this->enabledListener; l != NULL; l = l->next) {
		ActionEnabledListener * listener = (ActionEnabledListener *)l->data;
		listener->actionEnabledAction(action, enabled);
	}
}

void ActionHandler::addListener(ActionEnabledListener * listener) {
	this->enabledListener = g_list_append(this->enabledListener, listener);
}

void ActionHandler::removeListener(ActionEnabledListener * listener) {
	this->enabledListener = g_list_remove(this->enabledListener, listener);
}

void ActionHandler::fireActionSelected(ActionGroup group, ActionType action) {
	for(GList * l = this->selectionListener; l != NULL; l = l->next) {
		ActionSelectionListener * listener = (ActionSelectionListener *)l->data;
		listener->actionSelected(group, action);
	}
}

void ActionHandler::addListener(ActionSelectionListener * listener) {
	this->selectionListener = g_list_append(this->selectionListener, listener);
}

void ActionHandler::removeListener(ActionSelectionListener * listener) {
	this->selectionListener = g_list_remove(this->selectionListener, listener);
}

