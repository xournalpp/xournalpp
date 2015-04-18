/*
 * Xournal++
 *
 * The page format dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "../GladeGui.h"
#include "../../control/settings/Settings.h"
#include <XournalType.h>

enum Orientation
{
	ORIENTATION_NOT_DEFINED, ORIENTATION_LANDSCAPE, ORIENTATION_PORTRAIT
};

class FormatDialog : public GladeGui
{
public:
	FormatDialog(GladeSearchpath* gladeSearchPath, Settings* settings, double width,
				double heigth);
	virtual ~FormatDialog();

public:
	virtual void show(GtkWindow* parent);

	double getWidth();
	double getHeight();

private:
	void setOrientation(Orientation portrait);

	static void portraitSelectedCb(GtkToggleToolButton* toggle_tool_button,
								FormatDialog* dlg);
	static void landscapeSelectedCb(GtkToggleToolButton* toggle_tool_button,
									FormatDialog* dlg);
	static void cbFormatChangedCb(GtkComboBox* widget, FormatDialog* dlg);
	static void cbUnitChanged(GtkComboBox* widget, FormatDialog* dlg);
	static void spinValueChangedCb(GtkSpinButton* spinbutton, FormatDialog* dlg);

private:
	XOJ_TYPE_ATTRIB;


	Settings* settings;

	GList* list;

	Orientation orientation;
	double scale;
	int selectedScale;

	double origWidth;
	double origHeight;

	double width;
	double height;
};
