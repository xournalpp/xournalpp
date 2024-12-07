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

#define move_binding(mod, key, mvt, dir)                                                       \
    {hash(mod, GDK_KEY_##key), wrap<&TextEditor::moveCursor, mvt, dir, false>},                \
            {hash(mod & SHIFT, GDK_KEY_##key), wrap<&TextEditor::moveCursor, mvt, dir, true>}, \
            {hash(mod, GDK_KEY_KP_##key), wrap<&TextEditor::moveCursor, mvt, dir, false>}, {   \
        hash(mod& SHIFT, GDK_KEY_KP_##key), wrap<&TextEditor::moveCursor, mvt, dir, true>      \
    }

using KeyBinding::hash;
using KeyBinding::wrap;

const KeyBindingsGroup<TextEditor> TextEditor::keyBindings(
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


         {hash(CTRL, GDK_KEY_a), wrap<&TextEditor::selectAtCursor, TextEditor::SelectType::ALL>},
         {hash(CTRL, GDK_KEY_slash), wrap<&TextEditor::selectAtCursor, TextEditor::SelectType::ALL>},

         {hash(NONE, GDK_KEY_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_CHARS, 1>},
         {hash(NONE, GDK_KEY_KP_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_CHARS, 1>},

         {hash(NONE, GDK_KEY_BackSpace), wrap<&TextEditor::backspace>},
         {hash(SHIFT, GDK_KEY_BackSpace), wrap<&TextEditor::backspace>},

         {hash(CTRL, GDK_KEY_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, 1>},
         {hash(CTRL, GDK_KEY_KP_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, 1>},
         {hash(CTRL, GDK_KEY_BackSpace), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_WORD_ENDS, -1>},

         {hash(CTRL & SHIFT, GDK_KEY_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, 1>},
         {hash(CTRL & SHIFT, GDK_KEY_KP_Delete), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, 1>},
         {hash(CTRL & SHIFT, GDK_KEY_BackSpace), wrap<&TextEditor::deleteFromCursor, GTK_DELETE_PARAGRAPH_ENDS, -1>},

         {hash(CTRL, GDK_KEY_x), wrap<&TextEditor::cutToClipboard>},
         {hash(NONE, GDK_KEY_Cut), wrap<&TextEditor::cutToClipboard>},
         {hash(SHIFT, GDK_KEY_Delete), wrap<&TextEditor::cutToClipboard>},

         {hash(CTRL, GDK_KEY_c), wrap<&TextEditor::copyToClipboard>},
         {hash(NONE, GDK_KEY_Copy), wrap<&TextEditor::copyToClipboard>},
         {hash(CTRL, GDK_KEY_Insert), wrap<&TextEditor::copyToClipboard>},

         {hash(CTRL, GDK_KEY_v), wrap<&TextEditor::pasteFromClipboard>},
         {hash(NONE, GDK_KEY_Paste), wrap<&TextEditor::pasteFromClipboard>},
         {hash(SHIFT, GDK_KEY_Insert), wrap<&TextEditor::pasteFromClipboard>},

         {hash(NONE, GDK_KEY_Insert), wrap<&TextEditor::toggleOverwrite>},
         {hash(NONE, GDK_KEY_KP_Insert), wrap<&TextEditor::toggleOverwrite>},

         {hash(CTRL, GDK_KEY_b), wrap<&TextEditor::toggleBoldFace>},
         {hash(CTRL, GDK_KEY_B), wrap<&TextEditor::toggleBoldFace>},

         {hash(CTRL, GDK_KEY_plus), wrap<&TextEditor::increaseFontSize>},
         {hash(CTRL, GDK_KEY_KP_Add), wrap<&TextEditor::increaseFontSize>},

         {hash(CTRL, GDK_KEY_minus), wrap<&TextEditor::decreaseFontSize>},
         {hash(CTRL, GDK_KEY_KP_Subtract), wrap<&TextEditor::decreaseFontSize>},

         {hash(NONE, GDK_KEY_Return), wrap<&TextEditor::linebreak>},
         {hash(NONE, GDK_KEY_ISO_Enter), wrap<&TextEditor::linebreak>},
         {hash(NONE, GDK_KEY_KP_Enter), wrap<&TextEditor::linebreak>},

         {hash(NONE, GDK_KEY_Tab), wrap<&TextEditor::tabulation>},
         {hash(NONE, GDK_KEY_KP_Tab), wrap<&TextEditor::tabulation>},
         {hash(NONE, GDK_KEY_ISO_Left_Tab), wrap<&TextEditor::tabulation>},
         // Backwards tabulation (deindentation) is not implemented. Do as forward tabulation
         {hash(SHIFT, GDK_KEY_Tab), wrap<&TextEditor::tabulation>},
         {hash(SHIFT, GDK_KEY_KP_Tab), wrap<&TextEditor::tabulation>},
         {hash(SHIFT, GDK_KEY_ISO_Left_Tab), wrap<&TextEditor::tabulation>}});
