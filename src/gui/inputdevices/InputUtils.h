/**
 * @file InputUtils.h
 * @author idotobi
 * @brief Helper functions in use in the different InputHandlers as for example StylusInputHandler.h
 *
 */

#pragma once

#include "control/settings/SettingsEnums.h"

// forward declaration to avoid cyclic dependencies
class ToolHandler;
class Settings;

class InputUtils {
public:
    /**
     * @brief Change the active tool based on the pressed button and its corresponding settings.
     * In case the current tool is already selected nothing is done.
     *
     * @param toolHandler the toolHandler containing the tools
     * @param settings settings storing the information for the buttons
     * @param button the button pressed
     * @return true if the tool was changed as a result of the function.
     * @return false if the tool was not changed as it already was pointing to the correct tool.
     */
    static bool applyButton(ToolHandler* toolHandler, Settings* settings, Button button);

    /**
     * @brief Warn the user in case of Questionable Settings in for the Touchscreen "Tool"
     * Warn the user in case all of the following are true:
     *  - Disable Drawing for this device is set for Touchscreen
     *  - Tool - don't change is set for Touchscreen
     *  - current Tool in the toolbar is a Drawing tool (i.e. Pen/Highlighter/Eraser)
     * As this results in xournalpp just using the currently selected tool to draw
     * which might be surprising to users. Hence it's logged as warning.
     *
     * @param toolHandler the toolHandler containing the tools
     * @param settings settings storing the information for the buttons
     */
    static void warnIfQuestionableTouchDrawingSettings(ToolHandler* toolHandler, Settings* settings);
};
