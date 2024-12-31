#include "ActionDatabase.h"

#include <type_traits>
#include <utility>

#include <glib.h>
#include <gobject/gsignal.h>

#include "control/Control.h"
#include "control/settings/ShortcutConfiguration.h"
#include "gui/MainWindow.h"
#include "util/GVariantTemplate.h"
#include "util/safe_casts.h"  // for to_underlying Todo(cpp20) remove

#include "ActionProperties.h"

#ifdef DEBUG_ACTION_DB
#define START_ROW "   * " << std::left << std::setw(30) << std::boolalpha << Action_toString(a)
#endif

template <Action a, class U = void>
struct InitiallyEnabled {
    static inline void setup(ActionDatabase* db, Control*) { db->enableAction(a, true); }
};
template <Action a>
struct InitiallyEnabled<a, std::void_t<decltype(&ActionProperties<a>::initiallyEnabled)>> {
    static inline void setup(ActionDatabase* db, Control* ctrl) {
        db->enableAction(a, ActionProperties<a>::initiallyEnabled(ctrl));
    }
};


class ActionDatabase::Populator {
    /// Choose the right action namespace
    template <Action a, class U = void>
    struct ActionNamespace {
        static constexpr auto ACTION_NAMESPACE = "win.";
        static void addToActionMap(ActionDatabase* db) {
            g_action_map_add_action(G_ACTION_MAP(db->win), G_ACTION(db->entries[a].action.get()));
        }
    };
    template <Action a>
    struct ActionNamespace<
            a, std::enable_if_t<std::is_same_v<typename ActionProperties<a>::app_namespace, std::true_type>, void>> {
        static constexpr auto ACTION_NAMESPACE = "app.";
        static void addToActionMap(ActionDatabase* db) {
            g_action_map_add_action(G_ACTION_MAP(gtk_window_get_application(GTK_WINDOW(db->win))),
                                    G_ACTION(db->entries[a].action.get()));
        }
    };


    template <Action a>
    static inline void finishSetup(ActionDatabase* db, const char* signal) {
        db->entries[a].signalId = g_signal_connect(G_OBJECT(db->entries[a].action.get()), signal,
                                                   G_CALLBACK(ActionProperties<a>::callback), db->control);
        ActionNamespace<a>::addToActionMap(db);
        db->entries[a].namespacedName = std::string(ActionNamespace<a>::ACTION_NAMESPACE) + Action_toString(a);
        InitiallyEnabled<a>::setup(db, db->control);
    }

    // Actions without state or parameter
    template <Action a, std::enable_if_t<!has_param<a>() && !has_state<a>(), bool> = true>
    static void assign(ActionDatabase* db) {

        ACTIONDB_PRINT_DEBUG(START_ROW << " |               |            |");

        db->entries[a].action.reset(g_simple_action_new(Action_toString(a), nullptr), xoj::util::adopt);

        finishSetup<a>(db, "activate");
    }

    // Actions with parameter but no state
    template <Action a, std::enable_if_t<has_param<a>() && !has_state<a>(), bool> = true>
    static void assign(ActionDatabase* db) {

        ACTIONDB_PRINT_DEBUG(START_ROW << " |               | type = \""
                                       << (const char*)gVariantType<typename ActionProperties<a>::parameter_type>()
                                       << "\" |");

        db->entries[a].action.reset(
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

        db->entries[a].action.reset(
                g_simple_action_new_stateful(Action_toString(a), nullptr,
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

        db->entries[a].action.reset(
                g_simple_action_new_stateful(Action_toString(a),
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

    populateImpl(std::make_index_sequence<xoj::to_underlying(Action::ENUMERATOR_COUNT)>(), db);
}

ActionDatabase::ActionDatabase(Control* control):
        control(control), win(GTK_APPLICATION_WINDOW(control->getWindow()->getWindow())) {
    Populator::populate(this);
}

ActionDatabase::~ActionDatabase() {
    for (auto&& e: entries) {
        /**
         * The GAction's might still be referenced elsewhere (e.g. by the GtkApplicationWindow), so we need to
         * disconnect the signals in case a callback is called and the Control instance has been destroyed
         */
        g_signal_handler_disconnect(e.action.get(), e.signalId);
    }
}


void ActionDatabase::enableAction(Action action, bool enable) {
    xoj_assert(entries[action].action);
    g_simple_action_set_enabled(entries[action].action.get(), enable);
    ACTIONDB_PRINT_DEBUG((enable ? "Enabling Action \"" : "Disabling Action\"") << Action_toString(action) << "\"");
}

auto ActionDatabase::getAction(Action a) const -> ActionRef { return entries[a].action; }

bool ActionDatabase::isActionEnabled(Action a) const {
    xoj_assert(entries[a].action);
    return g_action_get_enabled(G_ACTION(entries[a].action.get()));
}

void ActionDatabase::disableAll() {
    for (auto&& a: entries) {
        g_simple_action_set_enabled(a.action.get(), false);
    }
}

template <size_t... As>
static void resetEnableStatusImpl(std::index_sequence<As...>, ActionDatabase* db, Control* ctrl) {
    ((InitiallyEnabled<static_cast<Action>(As)>::setup(db, ctrl)), ...);
}

void ActionDatabase::resetEnableStatus() {
    resetEnableStatusImpl(std::make_index_sequence<xoj::to_underlying(Action::ENUMERATOR_COUNT)>(), this, control);
}

void ActionDatabase::setShortcuts(const ShortcutConfiguration& config) {
    const auto& shortcuts = config.getActionsShortcuts();
    for (auto&& a: shortcuts) {
        GStrvBuilder* builder = g_strv_builder_new();
        for (auto&& s: a.second) {
            g_strv_builder_take(builder, gtk_accelerator_name(s.keyval, s.mod.mod));
        }
        GStrv accs = g_strv_builder_unref_to_strv(builder);
        std::string act = this->entries[a.first.action].namespacedName;
        if (a.first.parameter) {
            act += "(uint64 " + std::to_string(a.first.parameter.value()) + ")";
        }
        gtk_application_set_accels_for_action(GTK_APPLICATION(gtk_window_get_application(control->getGtkWindow())),
                                              act.c_str(), accs);
        g_strfreev(accs);
    }
}
