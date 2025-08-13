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

#include <gdk/gdkkeysyms.h>

#include "control/KeyBindingsGroup.h"

#include "TextEditor.h"

#define move_binding(mod, key, mvt, dir)                                                   \
    {{mod, GDK_KEY_##key}, wrap<&TextEditor::moveCursor, mvt, dir, false>},                \
            {{mod & SHIFT, GDK_KEY_##key}, wrap<&TextEditor::moveCursor, mvt, dir, true>}, \
            {{mod, GDK_KEY_KP_##key}, wrap<&TextEditor::moveCursor, mvt, dir, false>}, {   \
        {mod & SHIFT, GDK_KEY_KP_##key}, wrap<&TextEditor::moveCursor, mvt, dir, true>     \
    }

using KeyBindingsUtil::wrap;

const KeyBindingsGroup<TextEditor>& TextEditor::getDefaultKeyBindings() {
    static KeyBindingsGroup<TextEditor> defaultKeyBindings = {
            move_binding(NONE, Right, GTK_MOVEMENT_VISUAL_POSITIONS, 1),
            move_binding(NONE, Left, GTK_MOVEMENT_VISUAL_POSITIONS, -1),
            move_binding(CTRL_OR_META, Right, GTK_MOVEMENT_WORDS, 1),
            move_binding(CTRL_OR_META, Left, GTK_MOVEMENT_WORDS, -1),
            move_binding(NONE, Up, GTK_MOVEMENT_DISPLAY_LINES, -1),
            move_binding(NONE, Down, GTK_MOVEMENT_DISPLAY_LINES, 1),
            move_binding(CTRL_OR_META, Up, GTK_MOVEMENT_PARAGRAPHS, -1),
            move_binding(CTRL_OR_META, Down, GTK_MOVEMENT_PARAGRAPHS, 1),
            move_binding(NONE, Home, GTK_MOVEMENT_DISPLAY_LINE_ENDS, -1),
            move_binding(NONE, End, GTK_MOVEMENT_DISPLAY_LINE_ENDS, 1),
            move_binding(CTRL_OR_META, Home, GTK_MOVEMENT_BUFFER_ENDS, -1),
            move_binding(CTRL_OR_META, End, GTK_MOVEMENT_BUFFER_ENDS, 1),
            move_binding(NONE, Page_Up, GTK_MOVEMENT_PAGES, -1),
            move_binding(NONE, Page_Down, GTK_MOVEMENT_PAGES, 1),
            move_binding(CTRL_OR_META, Page_Up, GTK_MOVEMENT_HORIZONTAL_PAGES, -1),
            move_binding(CTRL_OR_META, Page_Down, GTK_MOVEMENT_HORIZONTAL_PAGES, 1),


            {{CTRL_OR_META, GDK_KEY_a}, wrap<&TextEditor::selectAtCursor, TextEditor::SelectType::ALL>},
            {{CTRL_OR_META, GDK_KEY_slash}, wrap<&TextEditor::selectAtCursor, TextEditor::SelectType::ALL>},

            {{NONE, GDK_KEY_Delete}, wrap<&TextEditor::deleteFromCursor, GTK_DELETE_CHARS, 1>},
            {{NONE, GDK_KEY_KP_Delete}, wrap<&TextEditor::deleteFromCursor, GTK_DELETE_CHARS, 1>},

            {{NONE, GDK_KEY_BackSpace}, wrap<&TextEditor::backspace>},
            {{SHIFT, GDK_KEY_BackSpace}, wrap<&TextEditor::backspace>},

            {{CTRL_OR_META, GDK_KEY_Delete}, wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, 1>},
            {{CTRL_OR_META, GDK_KEY_KP_Delete}, wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, 1>},
            {{CTRL_OR_META, GDK_KEY_BackSpace}, wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, -1>},

            {{CTRL_OR_META & SHIFT, GDK_KEY_Delete}, wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, 1>},
            {{CTRL_OR_META & SHIFT, GDK_KEY_KP_Delete},
             wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, 1>},
            {{CTRL_OR_META & SHIFT, GDK_KEY_BackSpace},
             wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, -1>},

            {{CTRL_OR_META, GDK_KEY_x}, wrap<&TextEditor::cutToClipboard>},
            {{NONE, GDK_KEY_Cut}, wrap<&TextEditor::cutToClipboard>},
            {{SHIFT, GDK_KEY_Delete}, wrap<&TextEditor::cutToClipboard>},

            {{CTRL_OR_META, GDK_KEY_c}, wrap<&TextEditor::copyToClipboard>},
            {{NONE, GDK_KEY_Copy}, wrap<&TextEditor::copyToClipboard>},
            {{CTRL_OR_META, GDK_KEY_Insert}, wrap<&TextEditor::copyToClipboard>},

            {{CTRL_OR_META, GDK_KEY_v}, wrap<&TextEditor::pasteFromClipboard>},
            {{NONE, GDK_KEY_Paste}, wrap<&TextEditor::pasteFromClipboard>},
            {{SHIFT, GDK_KEY_Insert}, wrap<&TextEditor::pasteFromClipboard>},

            {{NONE, GDK_KEY_Insert}, wrap<&TextEditor::toggleOverwrite>},
            {{NONE, GDK_KEY_KP_Insert}, wrap<&TextEditor::toggleOverwrite>},

            {{CTRL_OR_META, GDK_KEY_b}, wrap<&TextEditor::toggleBoldFace>},
            {{CTRL_OR_META, GDK_KEY_B}, wrap<&TextEditor::toggleBoldFace>},

            {{CTRL_OR_META, GDK_KEY_plus}, wrap<&TextEditor::increaseFontSize>},
            {{CTRL_OR_META, GDK_KEY_KP_Add}, wrap<&TextEditor::increaseFontSize>},

            {{CTRL_OR_META, GDK_KEY_minus}, wrap<&TextEditor::decreaseFontSize>},
            {{CTRL_OR_META, GDK_KEY_KP_Subtract}, wrap<&TextEditor::decreaseFontSize>},

            {{NONE, GDK_KEY_Return}, wrap<&TextEditor::linebreak>},
            {{NONE, GDK_KEY_ISO_Enter}, wrap<&TextEditor::linebreak>},
            {{NONE, GDK_KEY_KP_Enter}, wrap<&TextEditor::linebreak>},

            {{NONE, GDK_KEY_Tab}, wrap<&TextEditor::tabulation>},
            {{NONE, GDK_KEY_KP_Tab}, wrap<&TextEditor::tabulation>},
            {{NONE, GDK_KEY_ISO_Left_Tab}, wrap<&TextEditor::tabulation>},
            // Backwards tabulation (deindentation) is not implemented. Do as forward tabulation
            {{SHIFT, GDK_KEY_Tab}, wrap<&TextEditor::tabulation>},
            {{SHIFT, GDK_KEY_KP_Tab}, wrap<&TextEditor::tabulation>},
            {{SHIFT, GDK_KEY_ISO_Left_Tab}, wrap<&TextEditor::tabulation>}};
    return defaultKeyBindings;
}
