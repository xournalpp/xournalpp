/*
 * Xournal++
 *
 * Helper class for Touch specific fixes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gdk/gdk.h>

class Settings;

class TouchHelper
{
public:
	TouchHelper(Settings* settings);
	virtual ~TouchHelper();

public:
	/**
	 * Reload settings
	 */
	void reload();

	/**
	 * An event from a device occurred
	 */
	void event(GdkDevice* device);

private:
	/**
	 * Called after the timeout
	 *
	 * @return true to call again
	 */
	static bool enableTimeout(TouchHelper* self);

	/**
	 * There was a pen event, restart the timer
	 */
	void penEvent();

	/**
	 * Enable touchscreen
	 */
	void enableTouch();

	/**
	 * Disable touchscreen
	 */
	void disableTouch();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * If touch disabling is enabled
	 */
	bool enabled;

	/**
	 * true if touch is enabled, false if disabled
	 */
	bool touchState;

	/**
	 * When the pen last was seen
	 */
	gint64 lastPenAction;

	/**
	 * Timeout in ms
	 */
	int disableTimeout;

	/**
	 * Settings
	 */
	Settings* settings;
};
