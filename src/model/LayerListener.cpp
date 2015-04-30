#include "LayerListener.h"

#include "Layer.h"

LayerListener::LayerListener()
{
	this->layer = NULL;
}

LayerListener::~LayerListener()
{
	unregisterListener();
}

void LayerListener::registerListener(Layer* layer)
{
	this->layer = layer;
	this->layer->addListener(this);
}

void LayerListener::unregisterListener()
{
	if (this->layer)
	{
		this->layer->removeListener(this);
		this->layer = NULL;
	}
}

void LayerListener::layerDeleted()
{
	this->layerDeletedCb(this->layer);
	this->layer = NULL;
}
