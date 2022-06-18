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

#include <gtk/gtk.h>  // for GtkWidget

#include "enums/ActionGroup.enum.h"  // for GROUP_NOGROUP, ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType

#include "AbstractItem.h"  // for AbstractItem

class ActionHandler;

/**
 * Menuitem handler
 */
class MenuItem: public AbstractItem {
public:
    MenuItem(ActionHandler* handler, GtkWidget* widget, ActionType type, ActionGroup group = GROUP_NOGROUP);
    ~MenuItem() override;

private:
};
