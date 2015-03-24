/*
 * Xournal++
 *
 * A listener for changes on a single layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __LAYERLISTENER_H__
#define __LAYERLISTENER_H__

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

#endif /* __LAYERLISTENER_H__ */
