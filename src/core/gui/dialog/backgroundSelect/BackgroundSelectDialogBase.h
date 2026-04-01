/*
 * Xournal++
 *
 * Base class for Background selection dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkWindow

#include "util/Util.h"     // for npos
#include "util/raii/GtkWindowUPtr.h"

class Document;
class Settings;
class BaseElementView;
class GladeSearchpath;

class BackgroundSelectDialogBase {
protected:
    BackgroundSelectDialogBase(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings, const char* title);
    ~BackgroundSelectDialogBase();

public:
    Settings* getSettings();
    void setSelected(size_t selected);

    inline GtkWindow* getWindow() const { return window.get(); }

protected:
    /// Adds the entry widgets
    void populate();
    void layout();

protected:
    xoj::util::GtkWindowUPtr window;

    Settings* settings = nullptr;
    GtkScrolledWindow* scrolledWindow = nullptr;
    GtkFixed* container = nullptr;  ///< Area where miniatures are layed out
    GtkWidget* okButton = nullptr;
    GtkBox* vbox = nullptr;  ///< Vertical GtkBox containing all.

    Document* doc = nullptr;

    /// Selection confirmed
    bool confirmed = false;

    /// Selected entry, none if npos
    size_t selected = npos;

    /// Entries to display
    std::vector<std::unique_ptr<BaseElementView>> entries;
};
