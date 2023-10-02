#include "ActionDatabase.h"

#include <type_traits>
#include <utility>

#include <gobject/gsignal.h>

#include "control/Control.h"
#include "gui/MainWindow.h"
#include "util/GVariantTemplate.h"
#include "util/safe_casts.h"  // for to_underlying Todo(cpp20) remove

#include "ActionProperties.h"

#ifdef DEBUG_ACTION_DB
#define START_ROW "   * " << std::left << std::setw(30) << std::boolalpha << Action_toString(a)
#endif

class ActionDatabase::Populator {
    /**
     * Accelerators<a>::setup(ctrl); will setup the accelerators listed in ActionProperties<a>::accelerators (if any)
     */
    template <Action a, class U = void>
    struct Accelerators {
        static inline void setup(Control*) {}
    };
    template <Action a>
    struct Accelerators<a, std::void_t<decltype(&ActionProperties<a>::accelerators)>> {
        static_assert(std::is_array_v<decltype(ActionProperties<a>::accelerators)>);
        static_assert(
                std::is_same_v<std::remove_extent_t<decltype(ActionProperties<a>::accelerators)>, const char* const>);
        static_assert(
                ActionProperties<a>::accelerators[std::extent_v<decltype(ActionProperties<a>::accelerators)> - 1] ==
                        nullptr,
                "ActionProperties<a>::accelerators must be null terminated");

        static inline void setup(Control* ctrl) {
            // Todo(cpp20) constexpr this concatenation
            std::string fullActionName = "win.";
            fullActionName += Action_toString(a);
            gtk_application_set_accels_for_action(
                    GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(ctrl->getWindow()->getWindow()))),
                    fullActionName.c_str(), ActionProperties<a>::accelerators);
        }
    };
    template <Action a, class U = void>
    struct InitiallyEnabled {
        static inline void setup(ActionDatabase*) {}
    };
    template <Action a>
    struct InitiallyEnabled<a, std::void_t<decltype(&ActionProperties<a>::initiallyEnabled)>> {
        static inline void setup(ActionDatabase* db) {
            db->enableAction(a, ActionProperties<a>::initiallyEnabled(db->control));
        }
    };

    template <Action a>
    static inline void finishSetup(ActionDatabase* db, const char* signal) {
        db->signalIds[a] = g_signal_connect(G_OBJECT(db->gActions[a].get()), signal,
                                            G_CALLBACK(ActionProperties<a>::callback), db->control);
        g_action_map_add_action(G_ACTION_MAP(db->win), G_ACTION(db->gActions[a].get()));
        Accelerators<a>::setup(db->control);
        InitiallyEnabled<a>::setup(db);
    }

    // Actions without state or parameter
    template <Action a, std::enable_if_t<!has_param<a>() && !has_state<a>(), bool> = true>
    static void assign(ActionDatabase* db) {

        ACTIONDB_PRINT_DEBUG(START_ROW << " |               |            |");

        db->gActions[a].reset(g_simple_action_new(Action_toString(a), nullptr), xoj::util::adopt);

        finishSetup<a>(db, "activate");
    }

    // Actions with parameter but no state
    template <Action a, std::enable_if_t<has_param<a>() && !has_state<a>(), bool> = true>
    static void assign(ActionDatabase* db) {

        ACTIONDB_PRINT_DEBUG(START_ROW << " |               | type = \""
                                       << (const char*)gVariantType<typename ActionProperties<a>::parameter_type>()
                                       << "\" |");

        db->gActions[a].reset(
                g_simple_action_new(Action_toString(a), gVariantType<typename ActionProperties<a>::parameter_type>()),
                xoj::util::adopt);

        finishSetup<a>(db, "activate");
    }

    // Actions with a state but no parameter
    template <Action a, std::enable_if_t<!has_param<a>() && has_state<a>(), bool> = true>
    static void assign(ActionDatabase* db) {

        ACTIONDB_PRINT_DEBUG(START_ROW << " | \""
                                       << (const char*)gVariantType<typename ActionProperties<a>::state_type>()
                                       << "\" = " << std::setw(7) << ActionProperties<a>::initialState(db->control)
                                       << " |            |");

        db->gActions[a].reset(g_simple_action_new_stateful(Action_toString(a), nullptr,
                                                           makeGVariant<typename ActionProperties<a>::state_type>(
                                                                   ActionProperties<a>::initialState(db->control))),
                              xoj::util::adopt);

        finishSetup<a>(db, "change-state");
    }

    // Actions with both state and parameter (with matching type)
    template <Action a, std::enable_if_t<has_param<a>() && has_state<a>(), bool> = true>
    static void assign(ActionDatabase* db) {
        static_assert(
                std::is_same_v<typename ActionProperties<a>::state_type, typename ActionProperties<a>::parameter_type>);

        ACTIONDB_PRINT_DEBUG(
                START_ROW << " | \"" << (const char*)gVariantType<typename ActionProperties<a>::state_type>()
                          << "\" = " << std::setw(7) << ActionProperties<a>::initialState(db->control) << " | type = \""
                          << (const char*)gVariantType<typename ActionProperties<a>::state_type>() << "\" |");

        db->gActions[a].reset(g_simple_action_new_stateful(Action_toString(a),
                                                           gVariantType<typename ActionProperties<a>::state_type>(),
                                                           makeGVariant<typename ActionProperties<a>::state_type>(
                                                                   ActionProperties<a>::initialState(db->control))),
                              xoj::util::adopt);

        finishSetup<a>(db, "change-state");
    }

    template <size_t... As>
    static void populateImpl(std::index_sequence<As...>, ActionDatabase* db) {
        ((assign<static_cast<Action>(As)>(db)), ...);
    }


public:
    static void populate(ActionDatabase* db);
};

void ActionDatabase::Populator::populate(ActionDatabase* db) {
    ACTIONDB_PRINT_DEBUG("Populating ActionDatabase:");
    ACTIONDB_PRINT_DEBUG("        ACTION NAME:                |  STATE INIT   | PARAM TYPE |");

    populateImpl(std::make_index_sequence<std::to_underlying(Action::_COUNT)>(), db);
}

ActionDatabase::ActionDatabase(Control* control):
        control(control), win(GTK_APPLICATION_WINDOW(control->getWindow()->getWindow())) {
    Populator::populate(this);
}

ActionDatabase::~ActionDatabase() {
    auto sig = signalIds.begin();
    for (auto& a: gActions) {
        /**
         * The GAction's might still be referenced elsewhere (e.g. by the GtkApplicationWindow), so we need to
         * disconnect the signals in case a callback is called and the Control instance has been destroyed
         */
        g_signal_handler_disconnect(a.get(), *sig++);
    }
}


void ActionDatabase::enableAction(Action action, bool enable) {
    xoj_assert(gActions[action]);
    g_simple_action_set_enabled(gActions[action].get(), enable);
    ACTIONDB_PRINT_DEBUG((enable ? "Enabling Action \"" : "Disabling Action\"") << Action_toString(action) << "\"");
}
