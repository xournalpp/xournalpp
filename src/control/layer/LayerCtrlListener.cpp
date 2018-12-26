#include "LayerCtrlListener.h"
#include "LayerController.h"

LayerCtrlListener::LayerCtrlListener()
 : handler(NULL)
{
	XOJ_INIT_TYPE(LayerCtrlListener);
}

LayerCtrlListener::~LayerCtrlListener()
{
	XOJ_CHECK_TYPE(LayerCtrlListener);

	unregisterListener();

	XOJ_RELEASE_TYPE(LayerCtrlListener);
}

void LayerCtrlListener::registerListener(LayerController* handler)
{
	XOJ_CHECK_TYPE(LayerCtrlListener);

	this->handler = handler;
	handler->addListener(this);
}

void LayerCtrlListener::unregisterListener()
{
	XOJ_CHECK_TYPE(LayerCtrlListener);

	if (this->handler)
	{
		this->handler->removeListener(this);
	}
}

void LayerCtrlListener::rebuildLayerMenu()
{
	XOJ_CHECK_TYPE(LayerCtrlListener);
}

void LayerCtrlListener::layerVisibilityChanged()
{
	XOJ_CHECK_TYPE(LayerCtrlListener);
}


