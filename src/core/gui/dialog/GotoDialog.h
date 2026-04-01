/*
 * Xournal++
 *
 * Goto-Page dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>

#include <gtk/gtk.h>

#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;

namespace xoj::popup {
class GotoDialog {
public:
    GotoDialog(GladeSearchpath* gladeSearchPath, size_t initialPage, size_t maxPage,
               std::function<void(size_t)> callback);
    ~GotoDialog();

public:
    inline GtkWindow* getWindow() const { return window.get(); }

private:
    xoj::util::GtkWindowUPtr window;
    GtkSpinButton* spinButton = nullptr;

    std::function<void(size_t)> callback;
};
};  // namespace xoj::popup
