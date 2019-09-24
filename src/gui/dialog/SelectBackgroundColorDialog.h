/*
 * Xournal++
 *
 * The about dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

class Control;

/**
 * Count of last background colors
 */
const int LAST_BACKGROUND_COLOR_COUNT = 9;


class SelectBackgroundColorDialog
{
public:
	SelectBackgroundColorDialog(Control* control);
	virtual ~SelectBackgroundColorDialog();

public:
	void show(GtkWindow* parent);

	/**
	 * Return the selected color as RGB, -1 if no color is selected
	 */
	int getSelectedColor();

private:
	void storeLastUsedValuesInSettings();

private:
	Control* control = nullptr;

	/**
	 * Last used background colors (stored in settings)
	 */
	GdkRGBA lastBackgroundColors[LAST_BACKGROUND_COLOR_COUNT];

	/**
	 * Selected color
	 */
	int selected = -1;
};
