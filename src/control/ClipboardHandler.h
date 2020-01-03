/*
 * Xournal++
 *
 * The main Control
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "tools/EditSelection.h"

#include "XournalType.h"

class ObjectInputStream;

class ClipboardListener {
public:
    virtual void clipboardCutCopyEnabled(bool enabled) = 0;
    virtual void clipboardPasteEnabled(bool enabled) = 0;
    virtual void clipboardPasteText(string text) = 0;
    virtual void clipboardPasteImage(GdkPixbuf* img) = 0;
    virtual void clipboardPasteXournal(ObjectInputStream& in) = 0;
    virtual void deleteSelection() = 0;

    virtual ~ClipboardListener();
};

class ClipboardHandler {
public:
    ClipboardHandler(ClipboardListener* listener, GtkWidget* widget);
    virtual ~ClipboardHandler();

public:
    bool paste();
    bool cut();
    bool copy();

    void setSelection(EditSelection* selection);

    void setCopyPasteEnabled(bool enabled);

private:
    static void ownerChangedCallback(GtkClipboard* clip, GdkEvent* event, ClipboardHandler* handler);
    void clipboardUpdated(GdkAtom atom);
    static void receivedClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
                                          ClipboardHandler* handler);

    static void pasteClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
                                       ClipboardHandler* handler);
    static void pasteClipboardImage(GtkClipboard* clipboard, GdkPixbuf* pixbuf, ClipboardHandler* handler);

    static void pasteClipboardText(GtkClipboard* clipboard, const gchar* text, ClipboardHandler* handler);

private:
    ClipboardListener* listener = nullptr;
    GtkClipboard* clipboard = nullptr;
    gulong hanlderId = -1;

    EditSelection* selection = nullptr;

    bool containsText = false;
    bool containsXournal = false;
    bool containsImage = false;
};
