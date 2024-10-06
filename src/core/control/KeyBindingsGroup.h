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
#include <unordered_map>

#include <gdk/gdkevents.h>
#include <gdk/gdkkeysyms.h>

#include "gui/inputdevices/InputEvents.h"
#include "util/gdk4_helper.h"

struct PressedModifier {
    constexpr explicit PressedModifier(GdkModifierType mod): mod(mod) {}
    constexpr PressedModifier operator&(PressedModifier o) const {
        return PressedModifier((GdkModifierType)(mod | o.mod));
    }

    GdkModifierType mod;
};
constexpr PressedModifier NONE{(GdkModifierType)0};
constexpr PressedModifier CTRL(GDK_CONTROL_MASK);
constexpr PressedModifier SHIFT(GDK_SHIFT_MASK);
constexpr PressedModifier ALT(GDK_MOD1_MASK);

namespace KeyBinding {
using hash_type = uint64_t;
using keyval_type = std::invoke_result_t<decltype(gdk_key_event_get_keyval), GdkEvent*>;
static_assert(std::is_same<keyval_type, guint>::value);
static_assert(sizeof(PressedModifier) + sizeof(keyval_type) == sizeof(hash_type));

constexpr inline hash_type hash(PressedModifier mod, keyval_type keyval) {
    return (static_cast<hash_type>(keyval) | (static_cast<hash_type>(mod.mod) << 32));
}
inline hash_type hash(const KeyEvent& e) { return hash(PressedModifier(e.state), e.keyval); }

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
};  // namespace KeyBinding

template <class CtrlClass>
struct KeyBindingsGroup {
    using mapped_type = void (*)(CtrlClass*);
    using value_type = std::pair<const KeyBinding::hash_type, mapped_type>;

    KeyBindingsGroup(std::initializer_list<value_type> list): table(list) {}

    bool processEvent(CtrlClass* te, const KeyEvent& e) const {
        auto it = table.find(KeyBinding::hash(e));
        if (it == table.end()) {
            return false;
        }
        it->second(te);
        return true;
    }

private:
    std::unordered_map<KeyBinding::hash_type, mapped_type> table;
};
