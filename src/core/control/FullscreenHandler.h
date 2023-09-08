/*
 * Xournal++
 *
 * Fullscreen handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>


class Settings;
class MainWindow;

class FullscreenHandler {
public:
    FullscreenHandler();
    virtual ~FullscreenHandler();

public:
    bool isFullscreen() const;
    void setFullscreen(MainWindow* win, bool enabled);

private:
    void enableFullscreen(MainWindow* win);
    void disableFullscreen(MainWindow* win);

private:
    /**
     * Fullscreen enabled
     */
    bool fullscreen = false;
};
