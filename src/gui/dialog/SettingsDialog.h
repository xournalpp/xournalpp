/*
 * Xournal++
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

#include "../GladeGui.h"
#include "../../control/settings/Settings.h"
#include <XournalType.h>

class SettingsDialog: public GladeGui
{
public:
	SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings);
	virtual ~SettingsDialog();

public:
	virtual void show(GtkWindow* parent);

	void save();

	void setDpi(int dpi);

	void toolboxToggled();

private:
	void load();
	void loadCheckbox(const char* name, gboolean value);
	bool getCheckbox(const char* name);

	String updateHideString(String hidden, bool hideMenubar, bool hideSidebar);

	void initMouseButtonEvents();
	void initMouseButtonEvents(const char* hbox, int button,
	                           bool withDevice = false);

	static gboolean zoomcallibSliderChanged(GtkRange* range, GtkScrollType scroll,
	                                        gdouble value, SettingsDialog* dlg);

	static void toolboxToggledCallback(GtkToggleButton* togglebutton,
	                                   SettingsDialog* sd);

private:
	XOJ_TYPE_ATTRIB;

	Settings* settings;
	GtkWidget* callib;
	int dpi;

	GList* buttonConfigs;
};

#endif /* __SETTINGSDIALOG_H__ */
