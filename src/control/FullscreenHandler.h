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

	void enableFullscreen(MainWindow* win, bool enabled, bool presentation = false);

private:
	void enableFullscreen(MainWindow* win, string hideWidgets);
	void hideWidget(MainWindow* win, string widgetName);
	void disableFullscreen(MainWindow* win);

private:
	XOJ_TYPE_ATTRIB;

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
