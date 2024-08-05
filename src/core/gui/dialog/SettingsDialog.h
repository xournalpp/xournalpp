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

#include <functional>
#include <string>  // for string
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkWindow

#include "audio/DeviceInfo.h"                    // for DeviceInfo
#include "control/tools/StrokeStabilizerEnum.h"  // for AveragingMethod, Pre...
#include "gui/Builder.h"
#include "util/raii/GtkWindowUPtr.h"

#include "ButtonConfigGui.h"
#include "DeviceClassConfigGui.h"
#include "LanguageConfigGui.h"
#include "LatexSettingsPanel.h"
#include "SettingsDialogPaletteTab.h"
#include "filesystem.h"  // for path

class Control;
class Settings;

struct Palette;

class SettingsDialog {
public:
    SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings, Control* control,
                   const std::vector<fs::path>& paletteDirectories, std::function<void()> callback);

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void save();
    void setDpi(int dpi);

    /**
     * Set active regions
     */
    void enableWithCheckbox(const std::string& checkbox, const std::string& widget);
    void disableWithCheckbox(const std::string& checkbox, const std::string& widget);
    void enableWithEnabledCheckbox(const std::string& checkbox, const std::string& widget);

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
    void loadCheckbox(const char* name, bool value);
    bool getCheckbox(const char* name);

    void loadSlider(const char* name, double value);
    double getSlider(const char* name);

    void initMouseButtonEvents(GladeSearchpath* gladeSearchPath);

    void showStabilizerAvMethodOptions(StrokeStabilizer::AveragingMethod method);
    void showStabilizerPreprocessorOptions(StrokeStabilizer::Preprocessor preprocessor);

    void setAudioRecordingFolder(fs::path folder);

private:
    Settings* settings = nullptr;
    Control* control = nullptr;
    GtkWidget* callib = nullptr;
    int dpi = 72;
    fs::path audioRecordingsFolder;
    std::vector<DeviceInfo> audioInputDevices;
    std::vector<DeviceInfo> audioOutputDevices;

    Builder builder;
    xoj::util::GtkWindowUPtr window;

    LanguageConfigGui languageConfig;
    std::vector<std::unique_ptr<ButtonConfigGui>> buttonConfigs;
    std::vector<DeviceClassConfigGui> deviceClassConfigs;

    LatexSettingsPanel latexPanel;
    SettingsDialogPaletteTab paletteTab;

    std::function<void()> callback;
};
