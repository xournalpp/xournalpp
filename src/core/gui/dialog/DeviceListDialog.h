/*
 * Xournal++
 *
 * Dialog listing known devices
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

#include "util/raii/GtkWindowUPtr.h"

class Settings;
class GladeSearchpath;
class DeviceClassConfigGui;

class DeviceListDialog {
public:
    DeviceListDialog(GladeSearchpath* gladeSearchPath, Settings* settings);

public:
    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void saveSettings();

private:
    Settings* settings;
    std::vector<DeviceClassConfigGui> deviceClassConfigs;

    xoj::util::GtkWindowUPtr window;
};
