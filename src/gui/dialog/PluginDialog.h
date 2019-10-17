/*
 * Xournal++
 *
 * Plugin manage dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"

class PluginController;
class PluginDialogEntry;
class Settings;

class PluginDialog : public GladeGui
{
public:
	PluginDialog(GladeSearchpath* gladeSearchPath, Settings* settings);
	virtual ~PluginDialog();

public:
	void loadPluginList(PluginController* pc);
	virtual void show(GtkWindow* parent);

private:
	void saveSettings();

private:
	Settings* settings;

	vector<PluginDialogEntry*> plugins;
};
