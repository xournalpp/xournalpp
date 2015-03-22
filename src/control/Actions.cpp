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
		this->handler = handler;
	}
}

ActionHandler::ActionHandler()
{
	XOJ_INIT_TYPE(ActionHandler);

	this->enabledListener = NULL;
	this->selectionListener = NULL;
}

ActionHandler::~ActionHandler()
{
	XOJ_CHECK_TYPE(ActionHandler);

	g_list_free(this->enabledListener);
	g_list_free(this->selectionListener);

	XOJ_RELEASE_TYPE(ActionHandler);
}

void ActionHandler::fireEnableAction(ActionType action, bool enabled)
{
	XOJ_CHECK_TYPE(ActionHandler);

	for (GList* l = this->enabledListener; l != NULL; l = l->next)
	{
		ActionEnabledListener* listener = (ActionEnabledListener*) l->data;
		listener->actionEnabledAction(action, enabled);
	}
}

void ActionHandler::addListener(ActionEnabledListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->enabledListener = g_list_append(this->enabledListener, listener);
}

void ActionHandler::removeListener(ActionEnabledListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->enabledListener = g_list_remove(this->enabledListener, listener);
}

void ActionHandler::fireActionSelected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(ActionHandler);

	for (GList* l = this->selectionListener; l != NULL; l = l->next)
	{
		ActionSelectionListener* listener = (ActionSelectionListener*) l->data;
		listener->actionSelected(group, action);
	}
}

void ActionHandler::addListener(ActionSelectionListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->selectionListener = g_list_append(this->selectionListener, listener);
}

void ActionHandler::removeListener(ActionSelectionListener* listener)
{
	XOJ_CHECK_TYPE(ActionHandler);

	this->selectionListener = g_list_remove(this->selectionListener, listener);
}
