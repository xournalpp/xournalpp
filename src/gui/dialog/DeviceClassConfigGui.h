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

#include "control/Actions.h"
#include "gui/GladeGui.h"

#include <gdk/gdk.h>
#include <util/DeviceListHelper.h>

class Settings;
class SettingsDialog;

class DeviceClassConfigGui : public GladeGui
{
public:
	DeviceClassConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings, InputDevice device);
	virtual ~DeviceClassConfigGui();

public:
	void loadSettings();
	void saveSettings();

	// Not implemented! This is not a dialog!
	virtual void show(GtkWindow* parent);

private:
	static void cbSelectCallback(GtkComboBox* widget, DeviceClassConfigGui* gui);
	void enableDisableTools();

private:
	Settings* settings;
	InputDevice device;

	GtkWidget* labelDevice;
	GtkWidget* cbDeviceClass;
};
