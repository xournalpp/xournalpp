/*
 * Xournal++
 *
 * Handler for layer selection and hiding / showing layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "Actions.h"
#include "model/DocumentListener.h"

class LayerController : public DocumentListener
{
public:
	LayerController();
	virtual ~LayerController();

public:
	virtual void pageChanged(size_t page);

public:
	bool actionPerformed(ActionType type);

	/**
	 * Show all layer on the current page
	 */
	void showAllLayer();

	/**
	 * Hide all layer on the current page
	 */
	void hideAllLayer();

	void addNewLayer();
	void deleteCurrentLayer();
	void switchToLay(int layer);

private:
	XOJ_TYPE_ATTRIB;
};
