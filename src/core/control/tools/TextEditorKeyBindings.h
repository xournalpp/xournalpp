/*
 * Xournal++
 *
 * Key bindings for text editor event handling
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

#include "TextEditor.h"

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

struct TextEditor::KeyBindings {
    using hash_type = uint64_t;
    using keyval_type = std::invoke_result_t<decltype(gdk_key_event_get_keyval), GdkEvent*>;
    static_assert(std::is_same<keyval_type, guint>::value);
    using mapped_type = void (*)(TextEditor*);
    using value_type = std::pair<const hash_type, mapped_type>;
    static_assert(sizeof(PressedModifier) + sizeof(keyval_type) == sizeof(hash_type));

    KeyBindings(std::initializer_list<value_type> list): table(list) {}

    static constexpr hash_type hash(PressedModifier mod, keyval_type keyval) {
        return (static_cast<hash_type>(keyval) | (static_cast<hash_type>(mod.mod) << 32));
    }

    static hash_type hash(const KeyEvent& e) { return hash(PressedModifier(e.state), e.keyval); }
    std::unordered_map<hash_type, mapped_type> table;

    bool processEvent(TextEditor* te, const KeyEvent& e) const {
        auto it = table.find(hash(e));
        if (it == table.end()) {
            return false;
        }
        it->second(te);
        return true;
    }
};

template <auto fun, auto... a>
void wrap(TextEditor* te) {
    (te->*fun)(a...);
}

#define move_binding(mod, key, mvt, dir)                                                                    \
    {KeyBindings::hash(mod, GDK_KEY_##key), wrap<&TextEditor::moveCursor, mvt, dir, false>},                \
            {KeyBindings::hash(mod & SHIFT, GDK_KEY_##key), wrap<&TextEditor::moveCursor, mvt, dir, true>}, \
            {KeyBindings::hash(mod, GDK_KEY_KP_##key), wrap<&TextEditor::moveCursor, mvt, dir, false>}, {   \
        KeyBindings::hash(mod& SHIFT, GDK_KEY_KP_##key), wrap<&TextEditor::moveCursor, mvt, dir, true>      \
    }

const TextEditor::KeyBindings TextEditor::keyBindings(
        {move_binding(NONE, Right, GTK_MOVEMENT_VISUAL_POSITIONS, 1),
         move_binding(NONE, Left, GTK_MOVEMENT_VISUAL_POSITIONS, -1),
         move_binding(CTRL, Right, GTK_MOVEMENT_WORDS, 1),
         move_binding(CTRL, Left, GTK_MOVEMENT_WORDS, -1),
         move_binding(NONE, Up, GTK_MOVEMENT_DISPLAY_LINES, -1),
         move_binding(NONE, Down, GTK_MOVEMENT_DISPLAY_LINES, 1),
         move_binding(CTRL, Up, GTK_MOVEMENT_PARAGRAPHS, -1),
         move_binding(CTRL, Down, GTK_MOVEMENT_PARAGRAPHS, 1),
         move_binding(NONE, Home, GTK_MOVEMENT_DISPLAY_LINE_ENDS, -1),
         move_binding(NONE, End, GTK_MOVEMENT_DISPLAY_LINE_ENDS, 1),
         move_binding(CTRL, Home, GTK_MOVEMENT_BUFFER_ENDS, -1),
         move_binding(CTRL, End, GTK_MOVEMENT_BUFFER_ENDS, 1),
         move_binding(NONE, Page_Up, GTK_MOVEMENT_PAGES, -1),
         move_binding(NONE, Page_Down, GTK_MOVEMENT_PAGES, 1),
         move_binding(CTRL, Page_Up, GTK_MOVEMENT_HORIZONTAL_PAGES, -1),
         move_binding(CTRL, Page_Down, GTK_MOVEMENT_HORIZONTAL_PAGES, 1),


         {KeyBindings::hash(CTRL, GDK_KEY_a), wrap<&TextEditor::selectAtCursor, TextEditor::SelectType::ALL>},
         {KeyBindings::hash(CTRL, GDK_KEY_slash), wrap<&TextEditor::selectAtCursor, TextEditor::SelectType::ALL>},

         {KeyBindings::hash(NONE, GDK_KEY_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_CHARS, 1>},
         {KeyBindings::hash(NONE, GDK_KEY_KP_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_CHARS, 1>},

         {KeyBindings::hash(NONE, GDK_KEY_BackSpace), wrap<&TextEditor::backspace>},
         {KeyBindings::hash(SHIFT, GDK_KEY_BackSpace), wrap<&TextEditor::backspace>},


         {KeyBindings::hash(CTRL, GDK_KEY_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, 1>},
         {KeyBindings::hash(CTRL, GDK_KEY_KP_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, 1>},
         {KeyBindings::hash(CTRL, GDK_KEY_BackSpace), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, -1>},

         {KeyBindings::hash(CTRL & SHIFT, GDK_KEY_Delete),
          wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, 1>},
         {KeyBindings::hash(CTRL & SHIFT, GDK_KEY_KP_Delete),
          wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, 1>},
         {KeyBindings::hash(CTRL & SHIFT, GDK_KEY_BackSpace),
          wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, -1>},

         {KeyBindings::hash(CTRL, GDK_KEY_x), wrap<&TextEditor::cutToClipboard>},
         {KeyBindings::hash(NONE, GDK_KEY_Cut), wrap<&TextEditor::cutToClipboard>},
         {KeyBindings::hash(SHIFT, GDK_KEY_Delete), wrap<&TextEditor::cutToClipboard>},

         {KeyBindings::hash(CTRL, GDK_KEY_c), wrap<&TextEditor::copyToClipboard>},
         {KeyBindings::hash(NONE, GDK_KEY_Copy), wrap<&TextEditor::copyToClipboard>},
         {KeyBindings::hash(CTRL, GDK_KEY_Insert), wrap<&TextEditor::copyToClipboard>},

         {KeyBindings::hash(CTRL, GDK_KEY_v), wrap<&TextEditor::pasteFromClipboard>},
         {KeyBindings::hash(NONE, GDK_KEY_Paste), wrap<&TextEditor::pasteFromClipboard>},
         {KeyBindings::hash(SHIFT, GDK_KEY_Insert), wrap<&TextEditor::pasteFromClipboard>},

         {KeyBindings::hash(NONE, GDK_KEY_Insert), wrap<&TextEditor::toggleOverwrite>},
         {KeyBindings::hash(NONE, GDK_KEY_KP_Insert), wrap<&TextEditor::toggleOverwrite>},

         {KeyBindings::hash(CTRL, GDK_KEY_b), wrap<&TextEditor::toggleBoldFace>},
         {KeyBindings::hash(CTRL, GDK_KEY_B), wrap<&TextEditor::toggleBoldFace>},

         {KeyBindings::hash(CTRL, GDK_KEY_plus), wrap<&TextEditor::increaseFontSize>},
         {KeyBindings::hash(CTRL, GDK_KEY_KP_Add), wrap<&TextEditor::increaseFontSize>},

         {KeyBindings::hash(CTRL, GDK_KEY_minus), wrap<&TextEditor::decreaseFontSize>},
         {KeyBindings::hash(CTRL, GDK_KEY_KP_Subtract), wrap<&TextEditor::decreaseFontSize>},

         {KeyBindings::hash(NONE, GDK_KEY_Return), wrap<&TextEditor::linebreak>},
         {KeyBindings::hash(NONE, GDK_KEY_ISO_Enter), wrap<&TextEditor::linebreak>},
         {KeyBindings::hash(NONE, GDK_KEY_KP_Enter), wrap<&TextEditor::linebreak>},

         {KeyBindings::hash(NONE, GDK_KEY_Tab), wrap<&TextEditor::tabulation>},
         {KeyBindings::hash(NONE, GDK_KEY_KP_Tab), wrap<&TextEditor::tabulation>},
         {KeyBindings::hash(NONE, GDK_KEY_ISO_Left_Tab), wrap<&TextEditor::tabulation>},
         // Backwards tabulation (deindentation) is not implemented. Do as forward tabulation
         {KeyBindings::hash(SHIFT, GDK_KEY_Tab), wrap<&TextEditor::tabulation>},
         {KeyBindings::hash(SHIFT, GDK_KEY_KP_Tab), wrap<&TextEditor::tabulation>},
         {KeyBindings::hash(SHIFT, GDK_KEY_ISO_Left_Tab), wrap<&TextEditor::tabulation>}});
