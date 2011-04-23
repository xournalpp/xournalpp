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

class Collaboration : public LayerListener {
public:
	Collaboration(Control * control);
	virtual ~Collaboration();

public:
	void start();

	virtual void layerDeletedCb();

	virtual void elementAdded(Element * e);
	virtual void elementRemoved(Element * e);

private:
	Control * control;
};

#endif /* __COLLABORATION_H__ */
