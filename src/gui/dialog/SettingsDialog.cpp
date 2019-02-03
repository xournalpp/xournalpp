#include "SettingsDialog.h"

#include "ButtonConfigGui.h"
#include "gui/widgets/ZoomCallib.h"

#include <config.h>
#include <Util.h>
#include <StringUtils.h>

SettingsDialog::SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings, Control* control)
 : GladeGui(gladeSearchPath, "settings.glade", "settingsDialog"),
   settings(settings),
   control(control),
   callib(zoomcallib_new()),
   dpi(72)
{
	XOJ_INIT_TYPE(SettingsDialog);

	GtkWidget* vbox = get("zoomVBox");
	g_return_if_fail(vbox != NULL);

	GtkWidget* slider = get("zoomCallibSlider");
	g_return_if_fail(slider != NULL);

	g_signal_connect(slider, "change-value", G_CALLBACK(
		+[](GtkRange* range, GtkScrollType scroll, gdouble value, SettingsDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, SettingsDialog);
			self->setDpi((int) value);
		}), this);

	g_signal_connect(get("cbAutosave"), "toggled", G_CALLBACK(
		+[](GtkToggleButton* togglebutton, SettingsDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, SettingsDialog);
			self->autosaveToggled();
		}), this);


	g_signal_connect(get("btTestEnable"), "clicked", G_CALLBACK(
		+[](GtkButton* bt, SettingsDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, SettingsDialog);
			system(gtk_entry_get_text(GTK_ENTRY(self->get("txtEnableTouchCommand"))));
		}), this);

	g_signal_connect(get("btTestDisable"), "clicked", G_CALLBACK(
		+[](GtkButton* bt, SettingsDialog* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, SettingsDialog);
			system(gtk_entry_get_text(GTK_ENTRY(self->get("txtDisableTouchCommand"))));
		}), this);

	gtk_box_pack_start(GTK_BOX(vbox), callib, false, true, 0);
	gtk_widget_show(callib);

	initMouseButtonEvents();
}

SettingsDialog::~SettingsDialog()
{
	XOJ_CHECK_TYPE(SettingsDialog);

	for (ButtonConfigGui* bcg : this->buttonConfigs)
	{
		delete bcg;
	}

	// DO NOT delete settings!
	this->settings = NULL;

	XOJ_RELEASE_TYPE(SettingsDialog);
}

void SettingsDialog::initMouseButtonEvents(const char* hbox, int button, bool withDevice)
{
	XOJ_CHECK_TYPE(SettingsDialog);

	this->buttonConfigs.push_back(new ButtonConfigGui(this, getGladeSearchPath(), get(hbox), settings, button, withDevice));
}

void SettingsDialog::initMouseButtonEvents()
{
	XOJ_CHECK_TYPE(SettingsDialog);

	initMouseButtonEvents("hboxMidleMouse", 1);
	initMouseButtonEvents("hboxRightMouse", 2);
	initMouseButtonEvents("hboxEraser", 0);
	initMouseButtonEvents("hboxTouch", 3, true);
	initMouseButtonEvents("hboxPenButton1", 5);
	initMouseButtonEvents("hboxPenButton2", 6);

	initMouseButtonEvents("hboxDefault", 4);
}

void SettingsDialog::setDpi(int dpi)
{
	XOJ_CHECK_TYPE(SettingsDialog);

	if (this->dpi == dpi)
	{
		return;
	}

	this->dpi = dpi;
	zoomcallib_set_val(ZOOM_CALLIB(callib), dpi);
}

void SettingsDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(SettingsDialog);

	load();

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	int res = gtk_dialog_run(GTK_DIALOG(this->window));

	if (res == 1)
	{
		this->save();
	}

	gtk_widget_hide(this->window);
}

void SettingsDialog::loadCheckbox(const char* name, gboolean value)
{
	XOJ_CHECK_TYPE(SettingsDialog);

	GtkToggleButton* b = GTK_TOGGLE_BUTTON(get(name));
	gtk_toggle_button_set_active(b, value);
}

bool SettingsDialog::getCheckbox(const char* name)
{
	XOJ_CHECK_TYPE(SettingsDialog);

	GtkToggleButton* b = GTK_TOGGLE_BUTTON(get(name));
	return gtk_toggle_button_get_active(b);
}

/**
 * Autosave was toggled, enable / disable autosave config
 */
void SettingsDialog::autosaveToggled()
{
	XOJ_CHECK_TYPE(SettingsDialog);

	GtkWidget* cbAutosave = get("cbAutosave");
	bool autosaveEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbAutosave));
	gtk_widget_set_sensitive(get("lbAutosaveTimeout"), autosaveEnabled);
	gtk_widget_set_sensitive(get("spAutosaveTimeout"), autosaveEnabled);
}

void SettingsDialog::load()
{
	XOJ_CHECK_TYPE(SettingsDialog);

	loadCheckbox("cbSettingPresureSensitivity", settings->isPresureSensitivity());
	loadCheckbox("cbEnableZoomGestures", settings->isZoomGesturesEnabled());
	loadCheckbox("cbShowSidebarRight", settings->isSidebarOnRight());
	loadCheckbox("cbShowScrollbarLeft", settings->isScrollbarOnLeft());
	loadCheckbox("cbAutoloadXoj", settings->isAutloadPdfXoj());
	loadCheckbox("cbAutosave", settings->isAutosaveEnabled());
	loadCheckbox("cbAddVerticalSpace", settings->getAddVerticalSpace());
	loadCheckbox("cbAddHorizontalSpace", settings->getAddHorizontalSpace());
	loadCheckbox("cbBigCursor", settings->isShowBigCursor());
	loadCheckbox("cbHighlightPosition", settings->isHighlightPosition());
	loadCheckbox("cbDarkTheme", settings->isDarkTheme());
	loadCheckbox("cbHideHorizontalScrollbar", settings->getScrollbarHideType() & SCROLLBAR_HIDE_HORIZONTAL);
	loadCheckbox("cbHideVerticalScrollbar", settings->getScrollbarHideType() & SCROLLBAR_HIDE_VERTICAL);
	loadCheckbox("cbTouchWorkaround", settings->isTouchWorkaround());

	GtkWidget* txtDefaultSaveName = get("txtDefaultSaveName");
	string txt = settings->getDefaultSaveName();
	gtk_entry_set_text(GTK_ENTRY(txtDefaultSaveName), txt.c_str());

	gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(get("fcAudioPath")), settings->getAudioFolder().c_str());

	GtkWidget* spAutosaveTimeout = get("spAutosaveTimeout");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAutosaveTimeout), settings->getAutosaveTimeout());

	GtkWidget* spSnapRotationTolerance = get("spSnapRotationTolerance");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapRotationTolerance),settings->getSnapRotationTolerance());

	GtkWidget* spSnapGridTolerance = get("spSnapGridTolerance");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapGridTolerance),settings->getSnapGridTolerance());

	GtkWidget* slider = get("zoomCallibSlider");

	this->setDpi(settings->getDisplayDpi());
	gtk_range_set_value(GTK_RANGE(slider), dpi);

	GdkRGBA color;
	Util::apply_rgb_togdkrgba(color, settings->getBorderColor());
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("colorBorder")), &color);
	Util::apply_rgb_togdkrgba(color, settings->getBackgroundColor());
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("colorBackground")), &color);
	Util::apply_rgb_togdkrgba(color, settings->getSelectionColor());
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("colorSelection")), &color);


	bool hideFullscreenMenubar = false;
	bool hideFullscreenSidebar = false;
	bool hidePresentationMenubar = false;
	bool hidePresentationSidebar = false;

	string hidden = settings->getFullscreenHideElements();

	for (string element : StringUtils::split(hidden, ','))
	{
		if (element == "mainMenubar")
		{
			hideFullscreenMenubar = true;
		}
		else if (element == "sidebarContents")
		{
			hideFullscreenSidebar = true;
		}
	}

	hidden = settings->getPresentationHideElements();
	for (string element : StringUtils::split(hidden, ','))
	{
		if (element == "mainMenubar")
		{
			hidePresentationMenubar = true;
		}
		else if (element == "sidebarContents")
		{
			hidePresentationSidebar = true;
		}
	}

	loadCheckbox("cbHideFullscreenMenubar", hideFullscreenMenubar);
	loadCheckbox("cbHideFullscreenSidebar", hideFullscreenSidebar);
	loadCheckbox("cbHidePresentationMenubar", hidePresentationMenubar);
	loadCheckbox("cbHidePresentationSidebar", hidePresentationSidebar);

	autosaveToggled();


	SElement& touch = settings->getCustomElement("touch");
	bool disablePen = false;
	touch.getBool("disableTouch", disablePen);
	loadCheckbox("cbDisableTouchOnPenNear", disablePen);

	string disableMethod;
	touch.getString("method", disableMethod);
	int methodId = 0;
	if (disableMethod == "X11")
	{
		methodId = 1;
	}
	else if (disableMethod == "custom")
	{
		methodId = 2;
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbTouchDisableMethod")), methodId);

	string cmd;
	touch.getString("cmdEnable", cmd);
	gtk_entry_set_text(GTK_ENTRY(get("txtEnableTouchCommand")), cmd.c_str());

	cmd = "";
	touch.getString("cmdDisable", cmd);
	gtk_entry_set_text(GTK_ENTRY(get("txtDisableTouchCommand")), cmd.c_str());

	int timeoutMs = 1000;
	touch.getInt("timeout", timeoutMs);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spTouchDisableTimeout")), timeoutMs / 1000.0);

    this->audioInputDevices = this->control->getAudioController()->getAudioRecorder()->getInputDevices();
    for (auto &audioInputDevice : this->audioInputDevices)
	{
    	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(get("cbAudioInputDevice")), "", audioInputDevice.getDeviceName().c_str());
	}
    for (int i = 0; i < this->audioInputDevices.size(); i++)
    {
    	if (this->audioInputDevices[i].getSelected())
		{
			gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioInputDevice")), i);
		}
    }

    this->audioOutputDevices = this->control->getAudioController()->getAudioPlayer()->getOutputDevices();
	for (auto &audioOutputDevice : this->audioOutputDevices)
	{
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(get("cbAudioOutputDevice")), "", audioOutputDevice.getDeviceName().c_str());
	}
	for (int i = 0; i < this->audioOutputDevices.size(); i++)
	{
		if (this->audioOutputDevices[i].getSelected())
		{
			gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioOutputDevice")), i);
		}
	}

	switch((int)settings->getAudioSampleRate())
	{
		default:
		case 44100:
			gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioSampleRate")), 0);
			break;
		case 96100:
			gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioSampleRate")), 1);
			break;
		case 192000:
			gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioSampleRate")), 2);
			break;
	}

}

string SettingsDialog::updateHideString(string hidden, bool hideMenubar, bool hideSidebar)
{
	XOJ_CHECK_TYPE(SettingsDialog);

	string newHidden = "";

	for (string element : StringUtils::split(hidden, ','))
	{
		if (element == "mainMenubar")
		{
			if (hideMenubar)
			{
				hideMenubar = false;
			}
			else
			{
				continue;
			}
		}
		else if (element == "sidebarContents")
		{
			if (hideSidebar)
			{
				hideSidebar = false;
			}
			else
			{
				continue;
			}
		}

		if (!newHidden.empty())
		{
			newHidden += ",";
		}
		newHidden += element;
	}

	if (hideMenubar)
	{
		if (!newHidden.empty())
		{
			newHidden += ",";
		}
		newHidden += "mainMenubar";
	}

	if (hideSidebar)
	{
		if (!newHidden.empty())
		{
			newHidden += ",";
		}
		newHidden += "sidebarContents";
	}

	return newHidden;
}

void SettingsDialog::save()
{
	XOJ_CHECK_TYPE(SettingsDialog);

	settings->transactionStart();

	settings->setPresureSensitivity(getCheckbox("cbSettingPresureSensitivity"));
	settings->setZoomGesturesEnabled(getCheckbox("cbEnableZoomGestures"));
	settings->setSidebarOnRight(getCheckbox("cbShowSidebarRight"));
	settings->setScrollbarOnLeft(getCheckbox("cbShowScrollbarLeft"));
	settings->setAutoloadPdfXoj(getCheckbox("cbAutoloadXoj"));
	settings->setAutosaveEnabled(getCheckbox("cbAutosave"));
	settings->setAddVerticalSpace(getCheckbox("cbAddVerticalSpace"));
	settings->setAddHorizontalSpace(getCheckbox("cbAddHorizontalSpace"));
	settings->setShowBigCursor(getCheckbox("cbBigCursor"));
	settings->setHighlightPosition(getCheckbox("cbHighlightPosition"));
	settings->setDarkTheme(getCheckbox("cbDarkTheme"));
	settings->setTouchWorkaround(getCheckbox("cbTouchWorkaround"));

	int scrollbarHideType = SCROLLBAR_HIDE_NONE;
	if (getCheckbox("cbHideHorizontalScrollbar"))
	{
		scrollbarHideType |= SCROLLBAR_HIDE_HORIZONTAL;
	}
	if (getCheckbox("cbHideVerticalScrollbar"))
	{
		scrollbarHideType |= SCROLLBAR_HIDE_VERTICAL;
	}
	settings->setScrollbarHideType((ScrollbarHideType)scrollbarHideType);

	GdkRGBA color;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("colorBorder")), &color);
	settings->setBorderColor(Util::gdkrgba_to_hex(color));

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("colorBackground")), &color);
	settings->setBackgroundColor(Util::gdkrgba_to_hex(color));

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("colorSelection")), &color);
	settings->setSelectionColor(Util::gdkrgba_to_hex(color));


	bool hideFullscreenMenubar = getCheckbox("cbHideFullscreenMenubar");
	bool hideFullscreenSidebar = getCheckbox("cbHideFullscreenSidebar");
	settings->setFullscreenHideElements(
			updateHideString(settings->getFullscreenHideElements(), hideFullscreenMenubar, hideFullscreenSidebar));

	bool hidePresentationMenubar = getCheckbox("cbHidePresentationMenubar");
	bool hidePresentationSidebar = getCheckbox("cbHidePresentationSidebar");
	settings->setPresentationHideElements(
			updateHideString(settings->getPresentationHideElements(), hidePresentationMenubar,
					hidePresentationSidebar));

	settings->setDefaultSaveName(gtk_entry_get_text(GTK_ENTRY(get("txtDefaultSaveName"))));
	char* uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(get("fcAudioPath")));
	if (uri != NULL)
	{
		settings->setAudioFolder(uri);
		g_free(uri);
	}

	GtkWidget* spAutosaveTimeout = get("spAutosaveTimeout");
	int autosaveTimeout = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAutosaveTimeout));
	settings->setAutosaveTimeout(autosaveTimeout);

	settings->setDisplayDpi(dpi);

	for (ButtonConfigGui* bcg : this->buttonConfigs)
	{
		bcg->saveSettings();
	}

	SElement& touch = settings->getCustomElement("touch");
	touch.setBool("disableTouch", getCheckbox("cbDisableTouchOnPenNear"));
	int touchMethod = gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbTouchDisableMethod")));

	switch (touchMethod)
	{
	case 1:
		touch.setString("method", "X11");
		break;
	case 2:
		touch.setString("method", "custom");
		break;
	case 0:
	default:
		touch.setString("method", "auto");
	}
	touch.setString("cmdEnable", gtk_entry_get_text(GTK_ENTRY(get("txtEnableTouchCommand"))));
	touch.setString("cmdDisable", gtk_entry_get_text(GTK_ENTRY(get("txtDisableTouchCommand"))));

	touch.setInt("timeout", (int)(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spTouchDisableTimeout"))) * 1000));

	settings->setSnapRotationTolerance((double)gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spSnapRotationTolerance"))));
	settings->setSnapGridTolerance((double)gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spSnapGridTolerance"))));

	settings->setAudioInputDevice((int) this->audioInputDevices[gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbAudioInputDevice")))].getIndex());
	settings->setAudioOutputDevice((int) this->audioOutputDevices[gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbAudioOutputDevice")))].getIndex());

	switch (gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbAudioSampleRate"))))
	{
		default:
		case 0:
			settings->setAudioSampleRate(44100.0);
			break;
		case 1:
			settings->setAudioSampleRate(96100.0);
			break;
		case 2:
			settings->setAudioSampleRate(192000.0);
			break;
	}

	settings->transactionEnd();
}
