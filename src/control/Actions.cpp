#include "Actions.h"

ActionEnabledListener::ActionEnabledListener()
{
	this->handler = NULL;

	XOJ_INIT_TYPE(ActionEnabledListener);
}

ActionEnabledListener::~ActionEnabledListener()
{
	XOJ_CHECK_TYPE(ActionEnabledListener);

	unregisterListener();

	XOJ_RELEASE_TYPE(ActionEnabledListener);
}

void ActionEnabledListener::registerListener(ActionHandler* handler)
{
	XOJ_CHECK_TYPE(ActionEnabledListener);

	if (this->handler == NULL)
	{
		this->handler = handler;
		this->handler->addListener(this);
	}
}

void ActionEnabledListener::unregisterListener()
{
	XOJ_CHECK_TYPE(ActionEnabledListener);

	if (this->handler)
	{
		this->handler->removeListener(this);
		this->handler = NULL;
	}
}

ActionSelectionListener::ActionSelectionListener()
{
	XOJ_INIT_TYPE(ActionSelectionListener);

	this->handler = NULL;
}

ActionSelectionListener::~ActionSelectionListener()
{
	XOJ_CHECK_TYPE(ActionSelectionListener);

	unregisterListener();

	XOJ_RELEASE_TYPE(ActionSelectionListener);
}

void ActionSelectionListener::registerListener(ActionHandler* handler)
{
	XOJ_CHECK_TYPE(ActionSelectionListener);

	if (this->handler == NULL)
	{
		this->handler = handler;
		handler->addListener(this);
	}
}

void ActionSelectionListener::unregisterListener()
{
	XOJ_CHECK_TYPE(ActionSelectionListener);

	if (this->handler != NULL)
	{
		handler->removeListener(this);
	}
}

ActionHandler::ActionHandler()
{
	XOJ_INIT_TYPE(ActionHandler);
}

ActionHandler::~ActionHandler()
{
	XOJ_CHECK_TYPE(ActionHandler);

	XOJ_RELEASE_TYPE(ActionHandler);
}

void ActionHandler::fireEnableAction(ActionType action, bool enabled)
{
	XOJ_CHECK_TYPE(ActionHandler);

	for (ActionEnabledListener* listener : this->enabledListener)
	{
		listener->actionEnabledAction(action, enabled);
	}
}

void ActionHandler::addListener(ActionEnabledListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->enabledListener.push_back(listener);
}

void ActionHandler::removeListener(ActionEnabledListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->enabledListener.remove(listener);
}

void ActionHandler::fireActionSelected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(ActionHandler);

	for (ActionSelectionListener* listener : this->selectionListener)
	{
		listener->actionSelected(group, action);
	}
}

void ActionHandler::addListener(ActionSelectionListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->selectionListener.push_back(listener);
}

void ActionHandler::removeListener(ActionSelectionListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->selectionListener.remove(listener);
}
