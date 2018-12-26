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

class LayerController
{
public:
	LayerController();
	virtual ~LayerController();

public:
	bool actionPerformed(ActionType type);

	void addNewLayer();
	void deleteCurrentLayer();
	void switchToLay(int layer);

private:
	XOJ_TYPE_ATTRIB;
};
