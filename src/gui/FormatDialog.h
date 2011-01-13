/*
 * Xournal++
 *
 * The page format dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __FORMATDIALOG_H__
#define __FORMATDIALOG_H__

#include "GladeGui.h"
#include "../control/Settings.h"

enum Orientation {
	ORIENTATION_NOT_DEFINED, ORIENTATION_LANDSCAPE, ORIENTATION_PORTRAIT
};

class FormatDialog: public GladeGui {
public:
	FormatDialog(Settings * settings, double width, double heigth);
	virtual ~FormatDialog();

	void show();

	double getWidth();
	double getHeight();

private:
	void setOrientation(Orientation portrait);

	static void portraitSelectedCb(GtkToggleToolButton *toggle_tool_button, FormatDialog * dlg);
	static void landscapeSelectedCb(GtkToggleToolButton *toggle_tool_button, FormatDialog * dlg);
	static void cbFormatChangedCb(GtkComboBox *widget, FormatDialog * dlg);
	static void cbUnitChanged(GtkComboBox * widget, FormatDialog * dlg);
	static void spinValueChangedCb(GtkSpinButton * spinbutton, FormatDialog * dlg);
private:
	Settings * settings;

	Orientation orientation;
	double scale;
	int selectedScale;

	double origWidth;
	double origHeight;

	double width;
	double height;
};

#endif /* __FORMATDIALOG_H__ */
