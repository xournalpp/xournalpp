#include "FullscreenHandler.h"

#include "gui/MainWindow.h"  // for MainWindow

using std::string;


FullscreenHandler::FullscreenHandler() {}

FullscreenHandler::~FullscreenHandler() = default;

auto FullscreenHandler::isFullscreen() const -> bool { return this->fullscreen; }

void FullscreenHandler::enableFullscreen(MainWindow* win) { gtk_window_fullscreen(GTK_WINDOW(win->getWindow())); }

void FullscreenHandler::disableFullscreen(MainWindow* win) { gtk_window_unfullscreen(GTK_WINDOW(win->getWindow())); }

void FullscreenHandler::setFullscreen(MainWindow* win, bool enabled) {
    if (enabled) {
        enableFullscreen(win);
    } else {
        disableFullscreen(win);
    }

    this->fullscreen = enabled;
}