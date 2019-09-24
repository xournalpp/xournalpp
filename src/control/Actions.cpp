#include "Actions.h"

ActionEnabledListener::ActionEnabledListener()
{
}

ActionEnabledListener::~ActionEnabledListener()
{
	unregisterListener();
}

void ActionEnabledListener::registerListener(ActionHandler* handler)
{
	if (this->handler == nullptr)
	{
		this->handler = handler;
		this->handler->addListener(this);
	}
}

void ActionEnabledListener::unregisterListener()
{
	if (this->handler)
	{
		this->handler->removeListener(this);
		this->handler = nullptr;
	}
}

ActionSelectionListener::ActionSelectionListener()
{
	this->handler = nullptr;
}

ActionSelectionListener::~ActionSelectionListener()
{
	unregisterListener();
}

void ActionSelectionListener::registerListener(ActionHandler* handler)
{
	if (this->handler == nullptr)
	{
		this->handler = handler;
		handler->addListener(this);
	}
}

void ActionSelectionListener::unregisterListener()
{
	if (this->handler != nullptr)
	{
		handler->removeListener(this);
	}
}

ActionHandler::ActionHandler()
{
}

ActionHandler::~ActionHandler()
{
}

void ActionHandler::fireEnableAction(ActionType action, bool enabled)
{
	for (ActionEnabledListener* listener : this->enabledListener)
	{
		listener->actionEnabledAction(action, enabled);
	}
}

void ActionHandler::addListener(ActionEnabledListener* listener)
{
	this->enabledListener.push_back(listener);
}

void ActionHandler::removeListener(ActionEnabledListener* listener)
{
	this->enabledListener.remove(listener);
}

void ActionHandler::fireActionSelected(ActionGroup group, ActionType action)
{
	for (ActionSelectionListener* listener : this->selectionListener)
	{
		listener->actionSelected(group, action);
	}
}

void ActionHandler::addListener(ActionSelectionListener* listener)
{
	this->selectionListener.push_back(listener);
}

void ActionHandler::removeListener(ActionSelectionListener* listener)
{
	this->selectionListener.remove(listener);
}
