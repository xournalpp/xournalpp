/*
 * Xournal++
 *
 * RAII wrapper for g_source
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <glib-2.0/glib.h>

namespace xoj::util {
struct GSourceURef {
    GSourceURef(unsigned int id = 0): id(id) {}
    GSourceURef(const GSourceURef&) = delete;
    GSourceURef(GSourceURef&&) = delete;  // Implement if needed
    GSourceURef& operator=(const GSourceURef&) = delete;
    GSourceURef& operator=(GSourceURef&&) = delete;  // Implement if needed
    GSourceURef& operator=(unsigned int newId) {
        if (id) {
            g_source_remove(id);
        }
        id = newId;
        return *this;
    }
    ~GSourceURef() {
        if (id) {
            g_source_remove(id);
        }
    }

    operator bool() const { return id != 0; }

    /// When the GSource is run
    void consume() { id = 0; }
    /// To cancel the callback
    void cancel() { *this = 0; }

private:
    unsigned int id = 0;  // handler id
};
};  // namespace xoj::util
