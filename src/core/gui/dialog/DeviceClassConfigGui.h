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

#include <gdk/gdk.h>

#include "control/Actions.h"
#include "control/DeviceListHelper.h"
#include "gui/GladeGui.h"

class Settings;
class SettingsDialog;

class DeviceClassConfigGui: public GladeGui {
public:
    DeviceClassConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings, const InputDevice& device);
    ~DeviceClassConfigGui() override;

public:
    void loadSettings();
    void saveSettings();

    // Not implemented! This is not a dialog!
    void show(GtkWindow* parent) override;

private:
    static void cbSelectCallback(GtkComboBox* widget, DeviceClassConfigGui* gui);
    void enableDisableTools();

private:
    Settings* settings;
    InputDevice device;

    GtkWidget* labelDevice;
    GtkWidget* cbDeviceClass;
};
