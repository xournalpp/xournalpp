/*
 * Xournal Extended
 *
 * Settings Dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SETTINGSDIALOG_H__
#define __SETTINGSDIALOG_H__

#include "GladeGui.h"
#include "../control/Settings.h"

class SettingsDialog: public GladeGui {
public:
	SettingsDialog(Settings * dlg);
	virtual ~SettingsDialog();


	void show();
	void save();

	void setDpi(int dpi);

	void toolboxToggled();
private:
	void load();
	void loadCheckbox(const char * name, gboolean value);
	gboolean getCheckbox(const char * name);

	void initMouseButtonEvents();
	void initMouseButtonEvents(const char * hbox, int button, bool withDevice = false);

	SettingsDialog(const SettingsDialog & dlg);
	void operator=(const SettingsDialog & dlg);

	static void btSettingsOkClicked(GtkButton * button, SettingsDialog * dlg);
	static gboolean zoomcallibSliderChanged(GtkRange *range, GtkScrollType scroll, gdouble value, SettingsDialog * dlg);

	static void toolboxToggledCallback(GtkToggleButton *togglebutton, SettingsDialog * sd);
private:
	Settings * settings;
	GtkWidget * callib;
	int dpi;

	GList * buttonConfigs;
};

#endif /* __SETTINGSDIALOG_H__ */
