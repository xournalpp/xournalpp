/*
 * Xournal++
 *
 * The configuration for a button in the Settings Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <map>     // for map, map<>::value_compare
#include <string>  // for string
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkComboBox, GtkWindow

#include "control/DeviceListHelper.h"  // for InputDevice
#include "control/ToolEnums.h"         // for ToolSize
#include "gui/IconNameHelper.h"        // for IconNameHelper

class Settings;
class GladeSearchpath;

class ButtonConfigGui {
public:
    ButtonConfigGui(GladeSearchpath* gladeSearchPath, GtkBox* box, Settings* settings, unsigned int button,
                    bool withDevice);

public:
    void loadSettings();
    void saveSettings();

private:
    static void cbSelectCallback(GtkComboBox* widget, ButtonConfigGui* gui);
    void enableDisableTools();
    std::string toolSizeToLabel(ToolSize size);

private:
    Settings* settings;

    GtkWidget* cbDevice;
    GtkWidget* cbDisableDrawing;

    GtkWidget* cbTool;
    GtkWidget* cbThickness;
    GtkWidget* colorButton;
    GtkWidget* cbEraserType;
    GtkWidget* cbDrawingType;
    GtkWidget* cbStrokeType;

    std::vector<InputDevice> deviceList;

    unsigned int button = 0;
    bool withDevice = false;

    typedef std::map<int, ToolSize> ToolSizeIndexMap;
    static ToolSizeIndexMap toolSizeIndexMap;
    IconNameHelper iconNameHelper;
};
