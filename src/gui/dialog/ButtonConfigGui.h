/*
 * Xournal++
 *
 * The configuration for a button in the Settings Dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __BUTTONCONFIGGUI__
#define __BUTTONCONFIGGUI__

#include "../../control/Actions.h"
#include <gdk/gdk.h>

class SettingsDialog;
class Settings;

class ButtonConfigGui {
public:
	ButtonConfigGui(SettingsDialog * dlg, GtkWidget * w, Settings * settings, int button, bool withDevice);
	void loadSettings();
	void saveSettings();

private:
	GtkWidget * newLabel(const char * text);
	static void cbSelectCallback(GtkComboBox *widget, ButtonConfigGui * gui);
	void enableDisableTools();

private:
	Settings * settings;
	int button;
	bool withDevice;

	GtkWidget * cbDevice;
	GtkWidget * cbDisableDrawing;

	GtkWidget * cbTool;
	GtkWidget * cbThikness;
	GtkWidget * colorButton;
	GtkWidget * cbEraserType;
	GtkWidget * cbDrawingType;
};

#endif /* __BUTTONCONFIGGUI__ */
