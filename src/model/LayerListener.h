/*
 * Xournal++
 *
 * A listener for changes on a single layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class Element;
class Layer;

class LayerListener
{
public:
	LayerListener();
	virtual ~LayerListener();

public:
	void registerListener(Layer* layer);
	void unregisterListener();

public:
	void layerDeleted();
	virtual void layerDeletedCb(Layer* layer) = 0;

	virtual void elementAdded(Element* e, Layer* layer) = 0;
	virtual void elementRemoved(Element* e, Layer* layer) = 0;

private:
	Layer* layer;
};
