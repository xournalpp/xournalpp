/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "AbstractItem.h"

/**
 * Menuitem handler
 */
class MenuItem : public AbstractItem
{
public:
	MenuItem(ActionHandler* handler, GtkWidget* widget, ActionType type, ActionGroup group = GROUP_NOGROUP);
	virtual ~MenuItem();

private:
	};
