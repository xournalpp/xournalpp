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
	AbstractItemSelectionData(AbstractToolItem * item, String toolbar, int id, GtkWidget * w) {
		this->item = item;
		this->toolbar = toolbar;
		this->id = id;
		this->w = w;
	}

	GtkWidget * w;
	AbstractToolItem * item;
	String toolbar;
	int id;
};

#endif /* __ABSTRACTITEMSELECTIONDATA_H__ */
