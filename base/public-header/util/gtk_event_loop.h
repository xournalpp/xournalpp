//
// Created by julius on 02.05.20.
//

#pragma once

#include <functional>

#include <gdk/gdk.h>

struct with_gtk_event_loop {
private:
    struct _callback_wrapper {
        template <typename Fn>
        explicit _callback_wrapper(Fn fn): cb(std::forward(fn)) {}

        std::function<void()> cb;
    };

    template <typename Fn>
    void async(Fn&& fn) {
        throw std::runtime_error{"not implemented"};
    }

    template <typename Fn>
    void post(Fn&& fn) {
        gdk_threads_add_idle(reinterpret_cast<GSourceFunc>(callback), new _callback_wrapper{std::forward(fn)});
    }

    void pause() {}
    void resume() {}
    void stop() {}

private:
    static bool callback(_callback_wrapper* wrapper) {
        wrapper->cb();
        delete wrapper;
        return false;
    }
};
