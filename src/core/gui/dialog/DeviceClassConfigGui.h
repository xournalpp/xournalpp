/*
 * Xournal++
 *
 * The configuration for a button in the Settings Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkWidget, GtkComboBox, GtkWindow

#include "control/DeviceListHelper.h"  // for InputDevice

class Settings;
class GladeSearchpath;

class DeviceClassConfigGui {
public:
    DeviceClassConfigGui(GladeSearchpath* gladeSearchPath, GtkBox* box, Settings* settings, bool showApplyBtn = false);
    ~DeviceClassConfigGui();

public:
    void loadSettings();
    void saveSettings();

    /// Set the device represented by the entry.
    /// Please call saveSettings() beforehand if the settings of the previous device need to be saved.
    void setDevice(const InputDevice& device);

    inline bool representsDevice(const InputDevice& dev) const { return dev == device; }

private:
    Settings* settings;
    InputDevice device;

    GtkWidget* labelDevice;
    GtkWidget* cbDeviceClass;
};
