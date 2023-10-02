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

#include "config-debug.h"  // for DEBUG_ACTION_DB

class Control;

#ifdef DEBUG_ACTION_DB
#include <iomanip>
#include <iostream>

#include "util/safe_casts.h"
#define ACTIONDB_PRINT_DEBUG(f) std::cout << f << std::endl
template <class T, typename = void>
struct to_stream {
    to_stream(const T& t): t(t) {}
    const T& t;
    friend std::ostream& operator<<(std::ostream& str, const to_stream& self) { return str << self.t; }
};
template <class T>
struct to_stream<T, std::enable_if_t<std::is_enum_v<T>, void>> {
    to_stream(const T& t): t(t) {}
    const T& t;
    friend std::ostream& operator<<(std::ostream& str, const to_stream& self) {
        return str << std::to_underlying(self.t);
    }
};
#else
#define ACTIONDB_PRINT_DEBUG(f)
#endif

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
        ACTIONDB_PRINT_DEBUG("Set action \"" << Action_toString(a) << "\" state to " << to_stream(state));
    }

    /**
     * @brief Change the action's state, triggering callbacks
     */
    template <typename state_type>
    inline void fireChangeActionState(Action a, state_type state) {
        xoj_assert(gActions[a]);
        g_action_change_state(G_ACTION(gActions[a].get()), makeGVariant(state));
        ACTIONDB_PRINT_DEBUG("Fire action \"" << Action_toString(a) << "\" state change to " << to_stream(state));
    }

    /**
     * @brief Activate the action, triggering callbacks
     */
    inline void fireActivateAction(Action a) {
        xoj_assert(gActions[a]);
        g_action_activate(G_ACTION(gActions[a].get()), nullptr);
        ACTIONDB_PRINT_DEBUG("Fire action \"" << Action_toString(a) << "\" activate");
    }
    template <typename param_type>
    inline void fireActivateAction(Action a, param_type param) {
        xoj_assert(gActions[a]);
        g_action_activate(G_ACTION(gActions[a].get()), makeGVariant(param));
        ACTIONDB_PRINT_DEBUG("Fire action \"" << Action_toString(a) << "\" activate with param " << to_stream(param));
    }

private:
    EnumIndexedArray<xoj::util::GObjectSPtr<GSimpleAction>, Action> gActions;
    EnumIndexedArray<gulong, Action> signalIds;
    Control* control;
    GtkApplicationWindow* win;

    class Populator;
    friend class Populator;
};
