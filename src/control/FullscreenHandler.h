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

#include <XournalType.h>

#include <gtk/gtk.h>

class Settings;
class MainWindow;

class FullscreenHandler
{
public:
	FullscreenHandler(Settings* settings);
	virtual ~FullscreenHandler();

public:
	bool isFullscreen();
	void setFullscreen(MainWindow* win, bool enabled);

private:
	void enableFullscreen(MainWindow* win);
	void hideWidget(MainWindow* win, string widgetName);
	void disableFullscreen(MainWindow* win);

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
	vector<GtkWidget*> hiddenFullscreenWidgets;
};
