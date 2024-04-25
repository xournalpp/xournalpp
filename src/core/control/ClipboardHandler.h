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
#include <gdk/gdk.h>                // for GdkEvent, GdkClipboard
#include <glib.h>                   // for gchar, gulong
#include <gtk/gtk.h>                // for GtkWidget

class ObjectInputStream;
class EditSelection;

class ClipboardListener {
public:
    virtual void clipboardCutCopyEnabled(bool enabled) = 0;
    virtual void clipboardPasteEnabled(bool enabled) = 0;
    virtual void clipboardPasteText(std::string text) = 0;
    virtual void clipboardPasteImage(GdkPixbuf* img) = 0;  ///< Data is owned by the caller
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

    void setCopyCutEnabled(bool enabled);

private:
    void checkFormats();

private:
    ClipboardListener* listener = nullptr;
    GdkClipboard* clipboard = nullptr;
    gulong handlerId = 0;

    EditSelection* selection = nullptr;

    bool containsText = false;
    bool containsXopp = false;
    bool containsImage = false;
};
