/*
 * Xournal Extended
 *
 * Settings Dialog
 *
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#include "SettingsDialog.h"
#include "../widgets/ZoomCallib.h"
#include "../../gettext.h"
#include "ButtonConfig.h"
#include "../../util/Util.h"
#include <string.h>

SettingsDialog::SettingsDialog(Settings * settings) :
	GladeGui("settings.glade", "settingsDialog") {
	this->settings = settings;
	this->dpi = 72;
	callib = zoomcallib_new();
	this->buttonConfigs = NULL;

	GtkWidget * btSettingsOk = get("btSettingsOk");

	GtkWidget * vbox = get("zoomVBox");
	g_return_if_fail(vbox != NULL);

	GtkWidget * slider = get("zoomCallibSlider");
	g_return_if_fail(slider != NULL);
	g_signal_connect(slider, "change-value", G_CALLBACK(&zoomcallibSliderChanged), this);

	g_signal_connect(get("cbSettingXinput"), "toggled", G_CALLBACK(&toolboxToggledCallback), this);
	g_signal_connect(get("cbSettingPresureSensitivity"), "toggled", G_CALLBACK(&toolboxToggledCallback), this);
	g_signal_connect(get("cbAutosave"), "toggled", G_CALLBACK(&toolboxToggledCallback), this);

	gtk_box_pack_start(GTK_BOX(vbox), callib, false, true, 0);
	gtk_widget_show(callib);

	initMouseButtonEvents();
}

SettingsDialog::~SettingsDialog() {
	for (GList * l = this->buttonConfigs; l != NULL; l = l->next) {
		delete (ButtonConfigGui *) l->data;
	}
	g_list_free(this->buttonConfigs);

	// DO NOT delete settings!
	this->settings = NULL;
}

gboolean SettingsDialog::zoomcallibSliderChanged(GtkRange *range, GtkScrollType scroll, gdouble value,
		SettingsDialog * dlg) {
	dlg->setDpi((int) value);

	return false;
}

void SettingsDialog::initMouseButtonEvents(const char * hbox, int button, bool withDevice) {
	this->buttonConfigs = g_list_append(this->buttonConfigs, new ButtonConfigGui(this, get(hbox), settings, button,
			withDevice));
}

void SettingsDialog::initMouseButtonEvents() {
	initMouseButtonEvents("hboxMidleMouse", 1);
	initMouseButtonEvents("hboxRightMouse", 2);
	initMouseButtonEvents("hboxEraser", 0);
	initMouseButtonEvents("hboxTouch", 3, true);
}

void SettingsDialog::setDpi(int dpi) {
	if (this->dpi == dpi) {
		return;
	}

	this->dpi = dpi;
	zoomcallib_set_val(ZOOM_CALLIB(callib), dpi);
}

void SettingsDialog::show() {
	load();

	int res = gtk_dialog_run(GTK_DIALOG(this->window));

	if (res == 1) {
		this->save();
	}

	gtk_widget_hide(this->window);
}
void SettingsDialog::loadCheckbox(const char * name, gboolean value) {
	GtkToggleButton * b = GTK_TOGGLE_BUTTON(get(name));
	gtk_toggle_button_set_active(b, value);
}

gboolean SettingsDialog::getCheckbox(const char * name) {
	GtkToggleButton * b = GTK_TOGGLE_BUTTON(get(name));
	return gtk_toggle_button_get_active(b);
}

void SettingsDialog::toolboxToggledCallback(GtkToggleButton *togglebutton, SettingsDialog * sd) {
	sd->toolboxToggled();
}

void SettingsDialog::toolboxToggled() {
	GtkToggleButton * cbSettingXinput = GTK_TOGGLE_BUTTON(get("cbSettingXinput"));
	GtkWidget* cbSettingPresureSensitivity = get("cbSettingPresureSensitivity");
	GtkWidget* labePresureSensitivity = get("labePresureSensitivity");
	GtkWidget * labeXInput = get("labeXInput");

	gboolean xInputEnabled = gtk_toggle_button_get_active(cbSettingXinput);

	if (!settings->isXInputAvailable()) {
		xInputEnabled = false;

		gtk_widget_set_sensitive(GTK_WIDGET(cbSettingXinput), xInputEnabled);
		gtk_widget_set_sensitive(labeXInput, xInputEnabled);
	}

	GtkWidget * cbAutosave = get("cbAutosave");
	bool autosaveEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbAutosave));
	gtk_widget_set_sensitive(get("lbAutosaveTimeout"), autosaveEnabled);
	gtk_widget_set_sensitive(get("spAutosaveTimeout"), autosaveEnabled);

	gtk_widget_set_sensitive(cbSettingPresureSensitivity, xInputEnabled);
	gtk_widget_set_sensitive(labePresureSensitivity, xInputEnabled);
}

void SettingsDialog::load() {
	loadCheckbox("cbSettingXinput", settings->isXinputEnabled());
	loadCheckbox("cbSettingPresureSensitivity", settings->isPresureSensitivity());
	loadCheckbox("cbShowSidebarRight", settings->isSidebarOnRight());
	loadCheckbox("cbShowScrollbarLeft", settings->isScrollbarOnLeft());
	loadCheckbox("cbAutoloadXoj", settings->isAutloadPdfXoj());
	loadCheckbox("cbAutosave", settings->isAutosaveEnabled());
	loadCheckbox("cbSettingScrollOutside", settings->isAllowScrollOutsideThePage());



	GtkWidget * txtDefaultSaveName = get("txtDefaultSaveName");
	const char * txt = settings->getDefaultSaveName().c_str();
	if (txt == NULL) {
		txt = "";
	}
	gtk_entry_set_text(GTK_ENTRY(txtDefaultSaveName), txt);

	GtkWidget * spAutosaveTimeout = get("spAutosaveTimeout");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAutosaveTimeout), settings->getAutosaveTimeout());

	GtkWidget * slider = get("zoomCallibSlider");

	this->setDpi(settings->getDisplayDpi());
	gtk_range_set_value(GTK_RANGE(slider), dpi);

	GtkWidget * colorBorder = get("colorBorder");
	GdkColor color = Util::intToGdkColor(settings->getSelectionColor());
	gtk_color_button_set_color(GTK_COLOR_BUTTON(colorBorder), &color);

	bool hideFullscreenMenubar = false;
	bool hideFullscreenSidebar = false;
	bool hidePresentationMenubar = false;
	bool hidePresentationSidebar = false;

	String hidden = settings->getFullscreenHideElements();
	const char * element;
	StringTokenizer tokenF(hidden, ',');
	element = tokenF.next();
	while (element) {
		if (!strcmp("mainMenubar", element)) {
			hideFullscreenMenubar = true;
		} else if (!strcmp("sidebarContents", element)) {
			hideFullscreenSidebar = true;
		}
		element = tokenF.next();
	}

	hidden = settings->getPresentationHideElements();
	StringTokenizer token(hidden, ',');
	element = token.next();
	while (element) {
		if (!strcmp("mainMenubar", element)) {
			hidePresentationMenubar = true;
		} else if (!strcmp("sidebarContents", element)) {
			hidePresentationSidebar = true;
		}
		element = token.next();
	}

	loadCheckbox("cbHideFullscreenMenubar", hideFullscreenMenubar);
	loadCheckbox("cbHideFullscreenSidebar", hideFullscreenSidebar);
	loadCheckbox("cbHidePresentationMenubar", hidePresentationMenubar);
	loadCheckbox("cbHidePresentationSidebar", hidePresentationSidebar);

	toolboxToggled();
}

String SettingsDialog::updateHideString(String hidden, bool hideMenubar, bool hideSidebar) {
	String newHidden = "";

	const char * element;
	StringTokenizer token(hidden, ',');
	element = token.next();
	while (element) {
		if (!strcmp("mainMenubar", element)) {
			if (hideMenubar) {
				hideMenubar = false;
			} else {
				element = token.next();
				continue;
			}
		} else if (!strcmp("sidebarContents", element)) {
			if (hideSidebar) {
				hideSidebar = false;
			} else {
				element = token.next();
				continue;
			}
		}

		if (!newHidden.isEmpty()) {
			newHidden += ",";
		}
		newHidden += element;

		element = token.next();
	}

	if (hideMenubar) {
		if (!newHidden.isEmpty()) {
			newHidden += ",";
		}
		newHidden += "mainMenubar";
	}

	if (hideSidebar) {
		if (!newHidden.isEmpty()) {
			newHidden += ",";
		}
		newHidden += "sidebarContents";
	}

	return newHidden;
}

void SettingsDialog::save() {
	settings->setXinputEnabled(getCheckbox("cbSettingXinput"));
	settings->setPresureSensitivity(getCheckbox("cbSettingPresureSensitivity"));
	settings->setSidebarOnRight(getCheckbox("cbShowSidebarRight"));
	settings->setScrollbarOnLeft(getCheckbox("cbShowScrollbarLeft"));
	settings->setAutoloadPdfXoj(getCheckbox("cbAutoloadXoj"));
	settings->setAutosaveEnabled(getCheckbox("cbAutosave"));
	settings->setAllowScrollOutsideThePage(getCheckbox("cbSettingScrollOutside"));


	GtkWidget * colorBorder = get("colorBorder");
	GdkColor color = { 0 };
	gtk_color_button_get_color(GTK_COLOR_BUTTON(colorBorder), &color);
	int selectionColor = Util::gdkColorToInt(color);
	settings->setSelectionColor(selectionColor);

	bool hideFullscreenMenubar = getCheckbox("cbHideFullscreenMenubar");
	bool hideFullscreenSidebar = getCheckbox("cbHideFullscreenSidebar");
	settings->setFullscreenHideElements(updateHideString(settings->getFullscreenHideElements(), hideFullscreenMenubar,
			hideFullscreenSidebar));

	bool hidePresentationMenubar = getCheckbox("cbHidePresentationMenubar");
	bool hidePresentationSidebar = getCheckbox("cbHidePresentationSidebar");
	settings->setPresentationHideElements(updateHideString(settings->getPresentationHideElements(),
			hidePresentationMenubar, hidePresentationSidebar));

	GtkWidget * txtDefaultSaveName = get("txtDefaultSaveName");
	const char * txt = gtk_entry_get_text(GTK_ENTRY(txtDefaultSaveName));
	settings->setDefaultSaveName(txt);

	GtkWidget * spAutosaveTimeout = get("spAutosaveTimeout");
	int autosaveTimeout = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAutosaveTimeout));
	settings->setAutosaveTimeout(autosaveTimeout);

	settings->setDisplayDpi(dpi);

	for (GList * l = this->buttonConfigs; l != NULL; l = l->next) {
		((ButtonConfigGui *) l->data)->saveSettings();
	}
}
