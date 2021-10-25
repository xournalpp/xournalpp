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


/// This function should not be used in new code, please port the old code.
auto wait_for_gtk_dialog_result(GtkDialog* dialog) -> int {
    std::promise<int> promise;
    auto future = promise.get_future();
    g_signal_connect(dialog, "result", GCallback(+[](GtkDialog* /*self*/, int result, std::promise<int>* promise) {
                         promise->set_value(result);
                     }),
                     &promise);
    do {
        g_main_context_iteration(g_main_context_default(), false);
    } while (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready);
    return future.get();
};
