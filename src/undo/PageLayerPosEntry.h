/*
 * Xournal++
 *
 * Position entry for undo / redo handling
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PAGELAYERPOSENTRY_H__
#define __PAGELAYERPOSENTRY_H__

template<class T>
class PageLayerPosEntry {
public:
	PageLayerPosEntry(Layer * layer, T * element, int pos) {
		this->element = element;
		this->pos = pos;
		this->layer = layer;
	}

	Layer * layer;
	T * element;
	int pos;

	static int cmp(PageLayerPosEntry<T> * a, PageLayerPosEntry<T> * b) {
		return a->pos - b->pos;
	}
};

#endif /* __PAGELAYERPOSENTRY_H__ */
