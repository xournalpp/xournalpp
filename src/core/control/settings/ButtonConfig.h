/*
 * Xournal++
 *
 * Configuration for Mouse Buttons, Eraser and default configuration
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "control/ToolEnums.h"               // for DrawingType, ToolType
#include "control/settings/SettingsEnums.h"  // for Button
#include "util/Color.h"                      // for Color

class ToolHandler;

class ButtonConfig {
public:
    ButtonConfig(ToolType action, Color color, ToolSize size, DrawingType drawingType, EraserType eraserMode);
    virtual ~ButtonConfig();

public:
    /**
     * @brief Apply Changes to the button tool in case "Don't Change" was selected for one of it's properties
     * This is usually used after switching to a button tool from the toolbar tool
     * Note: In case of the setting "Don't Change" for Size/DrawingType/EraserType the
     * Size/DrawingType/EraserType of the corresponding tool in the toolbar is used.
     *
     * @param toolHandler
     * @param button button to be applied
     * @return true if some action was selected
     * @return false if no action was selected
     */
    bool applyNoChangeSettings(ToolHandler* toolHandler, Button button);

    /**
     * @brief Initialize Button tool
     * In case of "No change" settings the property will remain in it's default value.
     * As this will be overwritten during `applyNoChangeSettings` this makes sense.
     *
     * @param toolHandler
     * @param button
     */
    void initButton(ToolHandler* toolHandler, Button button);

    /**
     * @brief Apply the Config to the toolbar Tool
     * This is used for applying the Default Button Config
     *
     * @param toolHandler
     */
    void applyConfigToToolbarTool(ToolHandler* toolHandler);


    ToolType getAction();
    bool getDisableDrawing() const;
    DrawingType getDrawingType();

private:
    ToolType action;
    Color color{};
    ToolSize size;
    EraserType eraserMode;
    DrawingType drawingType;

    bool disableDrawing;

public:
    std::string device;

    friend class Settings;
    friend class ButtonConfigGui;
};
