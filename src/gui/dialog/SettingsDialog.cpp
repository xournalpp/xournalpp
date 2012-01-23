#include <config.h>
#include <glib/gi18n-lib.h>

#include "SettingsDialog.h"
#include "../widgets/ZoomCallib.h"
#include "ButtonConfigGui.h"
#include <Util.h>
#include <string.h>

SettingsDialog::SettingsDialog(GladeSearchpath * gladeSearchPath, Settings * settings) :
	GladeGui(gladeSearchPath, "settings.glade", "settingsDialog") {

	XOJ_INIT_TYPE(SettingsDialog);

	this->settings = settings;
	this->dpi = 72;
	callib = zoomcallib_new();
	this->buttonConfigs = NULL;

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
	XOJ_CHECK_TYPE(SettingsDialog);

	for (GList * l = this->buttonConfigs; l != NULL; l = l->next) {
		delete (ButtonConfigGui *) l->data;
	}
	g_list_free(this->buttonConfigs);

	// DO NOT delete settings!
	this->settings = NULL;

	XOJ_RELEASE_TYPE(SettingsDialog);
}

gboolean SettingsDialog::zoomcallibSliderChanged(GtkRange * range, GtkScrollType scroll, gdouble value, SettingsDialog * dlg) {
	XOJ_CHECK_TYPE_OBJ(dlg, SettingsDialog);

	dlg->setDpi((int) value);

	return false;
}

void SettingsDialog::initMouseButtonEvents(const char * hbox, int button, bool withDevice) {
	XOJ_CHECK_TYPE(SettingsDialog);

	this->buttonConfigs = g_list_append(this->buttonConfigs, new ButtonConfigGui(this, get(hbox), settings, button, withDevice));
}

void SettingsDialog::initMouseButtonEvents() {
	XOJ_CHECK_TYPE(SettingsDialog);

	initMouseButtonEvents("hboxMidleMouse", 1);
	initMouseButtonEvents("hboxRightMouse", 2);
	initMouseButtonEvents("hboxEraser", 0);
	initMouseButtonEvents("hboxTouch", 3, true);

	initMouseButtonEvents("hboxDefault", 4);
}

void SettingsDialog::setDpi(int dpi) {
	XOJ_CHECK_TYPE(SettingsDialog);

	if (this->dpi == dpi) {
		return;
	}

	this->dpi = dpi;
	zoomcallib_set_val(ZOOM_CALLIB(callib), dpi);
}

void SettingsDialog::show(GtkWindow * parent) {
	XOJ_CHECK_TYPE(SettingsDialog);

	load();

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	int res = gtk_dialog_run(GTK_DIALOG(this->window));

	if (res == 1) {
		this->save();
	}

	gtk_widget_hide(this->window);
}

void SettingsDialog::loadCheckbox(const char * name, gboolean value) {
	XOJ_CHECK_TYPE(SettingsDialog);

	GtkToggleButton * b = GTK_TOGGLE_BUTTON(get(name));
	gtk_toggle_button_set_active(b, value);
}

bool SettingsDialog::getCheckbox(const char * name) {
	XOJ_CHECK_TYPE(SettingsDialog);

	GtkToggleButton * b = GTK_TOGGLE_BUTTON(get(name));
	return gtk_toggle_button_get_active(b);
}

void SettingsDialog::toolboxToggledCallback(GtkToggleButton * togglebutton, SettingsDialog * sd) {
	XOJ_CHECK_TYPE_OBJ(sd, SettingsDialog);

	sd->toolboxToggled();
}

void SettingsDialog::toolboxToggled() {
	XOJ_CHECK_TYPE(SettingsDialog);

	GtkToggleButton * cbSettingXinput = GTK_TOGGLE_BUTTON(get("cbSettingXinput"));
	GtkWidget* cbSettingPresureSensitivity = get("cbSettingPresureSensitivity");
	GtkWidget* labePresureSensitivity = get("labePresureSensitivity");
	GtkWidget* labeIgnorCoreEvents = get("labeIgnorCoreEvents");
	GtkWidget* cbIgnorCoreEvents = get("cbIgnorCoreEvents");
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
	gtk_widget_set_sensitive(labeIgnorCoreEvents, xInputEnabled);
	gtk_widget_set_sensitive(cbIgnorCoreEvents, xInputEnabled);
}

void SettingsDialog::load() {
	XOJ_CHECK_TYPE(SettingsDialog);

	loadCheckbox("cbSettingXinput", settings->isXinputEnabled());
	loadCheckbox("cbIgnorCoreEvents", settings->isIgnoreCoreEvents());
	loadCheckbox("cbSettingPresureSensitivity", settings->isPresureSensitivity());
	loadCheckbox("cbShowSidebarRight", settings->isSidebarOnRight());
	loadCheckbox("cbShowScrollbarLeft", settings->isScrollbarOnLeft());
	loadCheckbox("cbAutoloadXoj", settings->isAutloadPdfXoj());
	loadCheckbox("cbAutosave", settings->isAutosaveEnabled());
	loadCheckbox("cbSettingScrollOutside", settings->isAllowScrollOutsideThePage());
	loadCheckbox("cbBigCursor", settings->isShowBigCursor());

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
	XOJ_CHECK_TYPE(SettingsDialog);

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
	XOJ_CHECK_TYPE(SettingsDialog);

	settings->setXinputEnabled(getCheckbox("cbSettingXinput"));
	settings->setPresureSensitivity(getCheckbox("cbSettingPresureSensitivity"));
	settings->setIgnoreCoreEvents(getCheckbox("cbIgnorCoreEvents"));
	settings->setSidebarOnRight(getCheckbox("cbShowSidebarRight"));
	settings->setScrollbarOnLeft(getCheckbox("cbShowScrollbarLeft"));
	settings->setAutoloadPdfXoj(getCheckbox("cbAutoloadXoj"));
	settings->setAutosaveEnabled(getCheckbox("cbAutosave"));
	settings->setAllowScrollOutsideThePage(getCheckbox("cbSettingScrollOutside"));
	settings->setShowBigCursor(getCheckbox("cbBigCursor"));

	GtkWidget * colorBorder = get("colorBorder");
	GdkColor color = { 0 };
	gtk_color_button_get_color(GTK_COLOR_BUTTON(colorBorder), &color);
	int selectionColor = Util::gdkColorToInt(color);
	settings->setSelectionColor(selectionColor);

	bool hideFullscreenMenubar = getCheckbox("cbHideFullscreenMenubar");
	bool hideFullscreenSidebar = getCheckbox("cbHideFullscreenSidebar");
	settings->setFullscreenHideElements(updateHideString(settings->getFullscreenHideElements(), hideFullscreenMenubar, hideFullscreenSidebar));

	bool hidePresentationMenubar = getCheckbox("cbHidePresentationMenubar");
	bool hidePresentationSidebar = getCheckbox("cbHidePresentationSidebar");
	settings->setPresentationHideElements(updateHideString(settings->getPresentationHideElements(), hidePresentationMenubar, hidePresentationSidebar));

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
