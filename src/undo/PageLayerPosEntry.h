/*
 * Xournal++
 *
 * Position entry for undo / redo handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

template<class T>
class PageLayerPosEntry
{
public:
	PageLayerPosEntry(Layer* layer, T* element, int pos)
	{
		XOJ_INIT_TYPE(PageLayerPosEntry);
		
		this->element = element;
		this->pos = pos;
		this->layer = layer;
	}
	
	~PageLayerPosEntry()
	{
		XOJ_RELEASE_TYPE(PageLayerPosEntry);
	}

private:
	XOJ_TYPE_ATTRIB;

public:
	Layer* layer;
	T* element;
	int pos;

	static int cmp(PageLayerPosEntry<T>* a, PageLayerPosEntry<T>* b)
	{
		return a->pos - b->pos;
	}
};
