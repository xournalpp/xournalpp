/*
 * Xournal++
 *
 * Collaboration implementation
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __COLLABORATION_H__
#define __COLLABORATION_H__

#include "../model/LayerListener.h"

class Control;
class Layer;

class Collaboration : public LayerListener {
public:
	Collaboration(Control * control);
	virtual ~Collaboration();

public:
	void start();

	virtual void layerDeletedCb(Layer * layer);

	virtual void elementAdded(Element * e, Layer * layer);
	virtual void elementRemoved(Element * e, Layer * layer);

private:
	Control * control;
};

#endif /* __COLLABORATION_H__ */
