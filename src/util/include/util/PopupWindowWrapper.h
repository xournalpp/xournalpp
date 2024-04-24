/*
 * Xournal++
 *
 * PopupWindow base class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include <gtk/gtk.h>

#include "util/Assert.h"

namespace xoj::popup {

/**
 * @brief The class PopupWindowWrapper allows a safe non-blocking creation and display of a popup window.
 * It shows the popup (upon a call to show()) and tasks a callback function to actually delete the popup once it has
 * been closed by the user.
 */
template <class PopupType>
class PopupWindowWrapper {
public:
    PopupWindowWrapper() = delete;
    PopupWindowWrapper(const PopupWindowWrapper&) = delete;
    PopupWindowWrapper(PopupWindowWrapper&&) = delete;

    template <class... Args>
    PopupWindowWrapper(Args&&... args) {
        popup = new PopupType(std::forward<Args>(args)...);
    }
    ~PopupWindowWrapper() { delete popup; }

    void show(GtkWindow* parent) {
        gtk_window_set_transient_for(popup->getWindow(), parent);
        gtk_window_set_modal(popup->getWindow(), true);

        gtk_widget_show(GTK_WIDGET(popup->getWindow()));
        g_signal_connect(popup->getWindow(), "close-request", G_CALLBACK(+[](GtkWindow*, gpointer popup) -> gboolean {
                             delete reinterpret_cast<PopupType*>(popup);
                             return true;  // Block the default callback: we destroy the window via ~GtkWindowUPtr()
                         }),
                         popup);

        /*
         * The actual popup must outlive this wrapper (so the main loop can go on).
         * As a consequence, once the popup is shown, this wrapper does not own the popup anymore.
         * The popup will get destroy by the signal connected above.
         */
        popup = nullptr;
    }

    PopupType* getPopup() const {
        xoj_assert_message(popup, "Do not call getPopup() after show()!");
        return popup;
    }

private:
    PopupType* popup = nullptr;
};
};  // namespace xoj::popup
