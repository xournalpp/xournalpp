/*
 * Xournal++
 *
 * Settings Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/Control.h"
#include "control/settings/Settings.h"
#include "gui/GladeGui.h"
#include "util/audio/DeviceInfo.h"

#include "DeviceClassConfigGui.h"
#include "LanguageConfigGui.h"
#include "LatexSettingsPanel.h"

class ButtonConfigGui;

class SettingsDialog: public GladeGui {
public:
    SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings, Control* control);
    ~SettingsDialog() override;

public:
    void show(GtkWindow* parent) override;

    void save();

    void setDpi(int dpi);

    /**
     * Autosave was toggled, enable / disable autosave config
     */
    void enableWithCheckbox(const string& checkbox, const string& widget);
    void customHandRecognitionToggled();
    void customStylusIconTypeChanged();

private:
    void load();
    void loadCheckbox(const char* name, gboolean value);
    bool getCheckbox(const char* name);

    static string updateHideString(const string& hidden, bool hideMenubar, bool hideSidebar);

    void initMouseButtonEvents();
    void initMouseButtonEvents(const char* hbox, int button, bool withDevice = false);

    void initLanguageSettings();

private:
    Settings* settings = nullptr;
    Control* control = nullptr;
    GtkWidget* callib = nullptr;
    int dpi = 72;
    vector<DeviceInfo> audioInputDevices;
    vector<DeviceInfo> audioOutputDevices;

    std::unique_ptr<LanguageConfigGui> languageConfig;
    vector<ButtonConfigGui*> buttonConfigs;
    vector<DeviceClassConfigGui*> deviceClassConfigs;

    LatexSettingsPanel latexPanel;
};
