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
    DeviceClassConfigGui(GladeSearchpath* gladeSearchPath, GtkBox* box, Settings* settings, const InputDevice& device);

public:
    void loadSettings();
    void saveSettings();

private:
    static void cbSelectCallback(GtkComboBox* widget, DeviceClassConfigGui* gui);
    void enableDisableTools();

private:
    Settings* settings;
    InputDevice device;

    GtkWidget* labelDevice;
    GtkWidget* cbDeviceClass;
};
