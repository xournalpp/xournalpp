/*
 * Xournal++
 *
 * The about dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SELECTBACKGROUNDCOLORDIALOG_H__
#define __SELECTBACKGROUNDCOLORDIALOG_H__

#include "../GladeGui.h"

class Control;
class ColorEntry;

class SelectBackgroundColorDialog : public GladeGui
{
public:
	SelectBackgroundColorDialog(GladeSearchpath* gladeSearchPath, Control* control);
	virtual ~SelectBackgroundColorDialog();

public:
	virtual void show(GtkWindow* parent);

	int getSelectedColor();

	void showColorchooser();

private:
	void updateLastUsedColors();

	static void buttonSelectedCallback(GtkButton* button, ColorEntry* e);
	static void buttonCustomCallback(GtkButton* button,
									SelectBackgroundColorDialog* dlg);

private:
	XOJ_TYPE_ATTRIB;

	Control* control;

	int selected;

	GList* colors;

	GtkWidget* colorDlg;
};

#endif /* __SELECTBACKGROUNDCOLORDIALOG_H__ */
