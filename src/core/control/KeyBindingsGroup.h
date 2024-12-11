/*
 * Xournal++
 *
 * Groups for Key bindings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#include <gdk/gdkkeysyms.h>

#include "gui/inputdevices/InputEvents.h"
#include "util/gdk4_helper.h"

struct PressedModifier {
    constexpr explicit PressedModifier(GdkModifierType mod): mod(mod) {}
    constexpr PressedModifier operator&(PressedModifier o) const {
        return PressedModifier((GdkModifierType)(mod | o.mod));
    }
    constexpr bool operator==(const PressedModifier& o) const {
        return mod == o.mod;
    }

    GdkModifierType mod;
};
constexpr PressedModifier NONE{(GdkModifierType)0};
constexpr PressedModifier SHIFT(GDK_SHIFT_MASK);
constexpr PressedModifier ALT(GDK_MOD1_MASK);

#ifdef __APPLE__
constexpr PressedModifier CTRL_OR_META(GDK_META_MASK);  ///< META on MaOS, CTRL on other platforms
#else
constexpr PressedModifier CTRL_OR_META(GDK_CONTROL_MASK);  ///< META on MaOS, CTRL on other platforms
#endif

struct Shortcut {
    using keyval_type = std::invoke_result_t<decltype(gdk_key_event_get_keyval), GdkEvent*>;
    static_assert(std::is_same<keyval_type, guint>::value);

    constexpr bool operator==(const Shortcut& o) const { return mod == o.mod && keyval == o.keyval; }

    PressedModifier mod;
    keyval_type keyval;
};

template<>
struct std::hash<Shortcut> {
    using hash_type = std::size_t;
    static_assert(sizeof(PressedModifier) + sizeof(Shortcut::keyval_type) == sizeof(hash_type));

    constexpr std::size_t operator()(const Shortcut& s) const noexcept {
        return (static_cast<hash_type>(s.keyval) | (static_cast<hash_type>(s.mod.mod) << 32));
    }
};

template <class CtrlClass>
struct KeyBindingsGroup {
    using mapped_type = std::function<void(CtrlClass*)>;
    using value_type = std::pair<const Shortcut, mapped_type>;
    using table_type = std::unordered_map<Shortcut, mapped_type>;

    KeyBindingsGroup(std::initializer_list<value_type> list): table(list) {}
    KeyBindingsGroup(table_type&& table): table(std::move(table)) {}

    bool processEvent(CtrlClass* te, const KeyEvent& e) const {
        auto it = table.find(Shortcut{PressedModifier(e.state), e.keyval});
        if (it == table.end()) {
            return false;
        }
        it->second(te);
        return true;
    }

private:
    table_type table;
};

namespace KeyBindingsUtil {
    template <class U>
    struct parent_class {};
    template <class M, class T>
    struct parent_class<M T::*> {
        using type = T;
    };

    template <auto fun, auto... a>
    void wrap(typename parent_class<decltype(fun)>::type* te) {
        (te->*fun)(a...);
    }
}
