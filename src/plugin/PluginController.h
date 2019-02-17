/*
 * Xournal++
 *
 * Plugin main controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class Control;

class PluginController
{
public:
	PluginController(Control* control);
	virtual ~PluginController();

public:
	/**
	 * Load all plugins within this folder
	 *
	 * @param path The path which contains the plugin folders
	 */
	void loadPluginsFrom(string path);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The main controller
	 */
	Control* control;
};
