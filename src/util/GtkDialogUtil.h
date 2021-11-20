#pragma once

#include <future>

#include <gtk/gtk.h>

/*
 * Xournal++
 *
 * Helper functions to work with GtkDialog's
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */


inline static void modal_dialog_result_handler(GtkDialog* /*self*/, int result, std::promise<int>* promise) {
    promise->set_value(result);
}

inline static void modal_dialog_close_handler(GtkDialog* /*self*/, std::promise<int>* promise) {
    promise->set_value(GTK_RESPONSE_CLOSE);
}


/// This function should not be used in new code, please port the old code.
inline auto wait_for_gtk_dialog_result(GtkDialog* dialog) -> int {
    std::promise<int> promise;
    auto future = promise.get_future();
    g_signal_connect(dialog, "result", GCallback(modal_dialog_result_handler), &promise);
    g_signal_connect(dialog, "close", GCallback(modal_dialog_close_handler), &promise);
    g_signal_connect(dialog, "close-request", GCallback(modal_dialog_close_handler), &promise);
    gtk_widget_show(GTK_WIDGET(dialog));
    do {
        g_main_context_iteration(g_main_context_default() /* g_main_context_default() */, true);
    } while (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready);
    return future.get();
};
