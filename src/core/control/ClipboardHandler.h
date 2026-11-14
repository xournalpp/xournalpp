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

#include <limits>  // for numeric_limits
#include <string>  // for string

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gdk/gdk.h>                // for GdkAtom, GdkEvent
#include <glib.h>                   // for gchar, gulong
#include <gtk/gtk.h>                // for GtkClipboard, GtkSelectionData

class ObjectInputStream;
class EditSelection;

class ClipboardActionStatusListener {
public:
    virtual void clipboardCutCopyEnabled(bool enabled) = 0;
    virtual void clipboardPasteEnabled(bool enabled) = 0;

    virtual ~ClipboardActionStatusListener() = default;
};

class ClipboardListener {
public:
    virtual void clipboardPasteText(std::string text) = 0;
    virtual void clipboardPasteImage(GdkPixbuf* img) = 0;
    virtual void clipboardPasteXournal(ObjectInputStream& in) = 0;
    virtual EditSelection* clipboardGetSelection() const = 0;
    virtual void deleteSelection() = 0;

    virtual ~ClipboardListener() = default;
};

class ClipboardHandler final {
public:
    ClipboardHandler(ClipboardActionStatusListener* actions, GtkWidget* widget);
    ~ClipboardHandler();

public:
    bool paste();
    bool cut();
    bool copy();

    void setSelection(EditSelection* selection);

    void setCopyCutEnabled(bool enabled);

    void setListener(ClipboardListener* listener);

private:
    static void ownerChangedCallback(GtkClipboard* clip, GdkEvent* event, ClipboardHandler* handler);
    void clipboardUpdated(GdkAtom atom);
    static void receivedClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
                                          ClipboardHandler* handler);

    void dispatchPaste();

    static void pasteClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
                                       ClipboardHandler* handler);
    static void pasteClipboardImage(GtkClipboard* clipboard, GdkPixbuf* pixbuf, ClipboardHandler* handler);

    static void pasteClipboardText(GtkClipboard* clipboard, const gchar* text, ClipboardHandler* handler);

private:
    ClipboardListener* listener = nullptr;
    ClipboardActionStatusListener* actions = nullptr;
    GtkClipboard* clipboard = nullptr;
    gulong handlerId = 0;

    bool containsText = false;
    bool containsXournal = false;
    bool containsImage = false;
    bool pastePending = false;
};
