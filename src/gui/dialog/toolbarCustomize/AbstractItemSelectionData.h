/*
 * Xournal++
 *
 * Toolbar drag & drop helper class
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ABSTRACTITEMSELECTIONDATA_H__
#define __ABSTRACTITEMSELECTIONDATA_H__

#include <String.h>
#include <gtk/gtk.h>

class AbstractToolItem;

/**
 * Used for transport a Toolbar item
 */
class AbstractItemSelectionData {
public:
	AbstractItemSelectionData(AbstractToolItem * item, String toolbar, int position) {
		this->item = item;
		this->toolbar = toolbar;
		this->position = position;
	}

	AbstractToolItem * item;
	String toolbar;
	int position;
};

#endif /* __ABSTRACTITEMSELECTIONDATA_H__ */
