/*
 * Xournal++
 *
 * Settings Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <util/audio/DeviceInfo.h>
#include <control/Control.h>
#include "control/settings/Settings.h"
#include "gui/GladeGui.h"

class ButtonConfigGui;

class SettingsDialog : public GladeGui
{
public:
	SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings, Control* control);
	virtual ~SettingsDialog();

public:
	virtual void show(GtkWindow* parent);

	void save();

	void setDpi(int dpi);

	/**
	 * Autosave was toggled, enable / disable autosave config
	 */
	void autosaveToggled();

private:
	void load();
	void loadCheckbox(const char* name, gboolean value);
	bool getCheckbox(const char* name);

	string updateHideString(string hidden, bool hideMenubar, bool hideSidebar);

	void initMouseButtonEvents();
	void initMouseButtonEvents(const char* hbox, int button, bool withDevice = false);

private:
	XOJ_TYPE_ATTRIB;

	Settings* settings;
	Control* control;
	GtkWidget* callib;
	int dpi;
	vector<DeviceInfo> audioInputDevices;
	vector<DeviceInfo> audioOutputDevices;

	vector<ButtonConfigGui*> buttonConfigs;
};
