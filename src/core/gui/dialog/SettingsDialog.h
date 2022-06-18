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

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

#include <glib.h>     // for gboolean
#include <gtk/gtk.h>  // for GtkWidget, GtkWindow

#include "audio/DeviceInfo.h"                    // for DeviceInfo
#include "control/tools/StrokeStabilizerEnum.h"  // for AveragingMethod, Pre...
#include "gui/GladeGui.h"                        // for GladeGui

#include "LatexSettingsPanel.h"  // for LatexSettingsPanel

class ButtonConfigGui;
class Control;
class DeviceClassConfigGui;
class GladeSearchpath;
class LanguageConfigGui;
class Settings;

class SettingsDialog: public GladeGui {
public:
    SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings, Control* control);
    ~SettingsDialog() override;

public:
    void show(GtkWindow* parent) override;

    void save();

    void setDpi(int dpi);

    /**
     * Set active regions
     */
    void enableWithCheckbox(const std::string& checkbox, const std::string& widget);
    void disableWithCheckbox(const std::string& checkbox, const std::string& widget);

    /*
     * Listeners for changes to settings.
     */
    void customHandRecognitionToggled();
    void customStylusIconTypeChanged();

    /**
     * Update whether options can be selected, tooltips, etc. for
     * pressure sensitivity options (e.g. pressure multiplier).
     */
    void updatePressureSensitivityOptions();

private:
    void load();
    void loadCheckbox(const char* name, gboolean value);
    bool getCheckbox(const char* name);

    void loadSlider(const char* name, double value);
    double getSlider(const char* name);

    static std::string updateHideString(const std::string& hidden, bool hideMenubar, bool hideSidebar);

    void initMouseButtonEvents();
    void initMouseButtonEvents(const char* hbox, int button, bool withDevice = false);

    void initLanguageSettings();

    void showStabilizerAvMethodOptions(StrokeStabilizer::AveragingMethod method);
    void showStabilizerPreprocessorOptions(StrokeStabilizer::Preprocessor preprocessor);

private:
    Settings* settings = nullptr;
    Control* control = nullptr;
    GtkWidget* callib = nullptr;
    int dpi = 72;
    std::vector<DeviceInfo> audioInputDevices;
    std::vector<DeviceInfo> audioOutputDevices;

    std::unique_ptr<LanguageConfigGui> languageConfig;
    std::vector<ButtonConfigGui*> buttonConfigs;
    std::vector<DeviceClassConfigGui*> deviceClassConfigs;

    LatexSettingsPanel latexPanel;
};
