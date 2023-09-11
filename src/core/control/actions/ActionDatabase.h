/**
 *
 *
 */
#pragma once

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "enums/Action.enum.h"
#include "util/Assert.h"
#include "util/EnumIndexedArray.h"
#include "util/GVariantTemplate.h"
#include "util/raii/GObjectSPtr.h"

class Control;

class ActionDatabase {
public:
    ActionDatabase(Control* control);
    ~ActionDatabase();

    void enableAction(Action a, bool enable);

    /**
     * @brief Set the action's state, without triggering callbacks
     */
    template <typename state_type>
    inline void setActionState(Action a, state_type state) {
        xoj_assert(gActions[a]);
        g_simple_action_set_state(gActions[a].get(), makeGVariant(state));
    }

    /**
     * @brief Change the action's state, triggering callbacks
     */
    template <typename state_type>
    inline void fireChangeActionState(Action a, state_type state) {
        xoj_assert(gActions[a]);
        g_action_change_state(G_ACTION(gActions[a].get()), makeGVariant(state));
    }

    /**
     * @brief Activate the action, triggering callbacks
     */
    inline void fireActivateAction(Action a) {
        xoj_assert(gActions[a]);
        g_action_activate(G_ACTION(gActions[a].get()), nullptr);
    }
    template <typename param_type>
    inline void fireActivateAction(Action a, param_type param) {
        xoj_assert(gActions[a]);
        g_action_activate(G_ACTION(gActions[a].get()), makeGVariant(param));
    }

private:
    EnumIndexedArray<xoj::util::GObjectSPtr<GSimpleAction>, Action> gActions;
    EnumIndexedArray<gulong, Action> signalIds;
    Control* control;
    GtkApplicationWindow* win;

    class Populator;
    friend class Populator;
};
