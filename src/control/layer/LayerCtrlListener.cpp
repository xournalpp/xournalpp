#include "LayerCtrlListener.h"
#include "LayerController.h"

LayerCtrlListener::LayerCtrlListener()
 : handler(nullptr)
{
}

LayerCtrlListener::~LayerCtrlListener()
{
	unregisterListener();
}

void LayerCtrlListener::registerListener(LayerController* handler)
{
	this->handler = handler;
	handler->addListener(this);
}

void LayerCtrlListener::unregisterListener()
{
	if (this->handler)
	{
		this->handler->removeListener(this);
	}
}

void LayerCtrlListener::rebuildLayerMenu()
{
}

void LayerCtrlListener::layerVisibilityChanged()
{
}


