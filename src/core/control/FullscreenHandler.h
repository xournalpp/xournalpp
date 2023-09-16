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

#include "util/raii/GObjectSPtr.h"


class Settings;
class Control;

class FullscreenHandler {
public:
    FullscreenHandler(Settings* settings);
    virtual ~FullscreenHandler();

public:
    bool isFullscreen() const;
    void setFullscreen(Control* ctrl, bool enabled);

private:
    void enableFullscreen(Control* ctrl);
    void disableFullscreen(Control* ctrl);

private:
    /**
     * Settings
     */
    Settings* settings;

    /**
     * Fullscreen enabled
     */
    bool fullscreen = false;

    /**
     * If the sidebar was hidden
     */
    bool sidebarHidden = false;

    /**
     * The menubar was hidden
     */
    bool menubarHidden = false;

    /**
     * Currently hidden widgets
     */
    std::vector<xoj::util::WidgetSPtr> hiddenFullscreenWidgets;
};
