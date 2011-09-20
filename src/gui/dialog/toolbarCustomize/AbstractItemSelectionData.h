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

class AbstractToolItem;

/**
 * Used for transport a Toolbar item
 */
class AbstractItemSelectionData {
public:
	AbstractItemSelectionData(AbstractToolItem * item) {
		this->item = item;
	}

	AbstractToolItem * item;
};

#endif /* __ABSTRACTITEMSELECTIONDATA_H__ */
