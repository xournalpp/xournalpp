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

#include <string>
#include <vector>

#include "control/Tool.h"

#include "XournalType.h"

class ToolHandler;

class ButtonConfig {
public:
    ButtonConfig(ToolType action, Color color, ToolSize size, DrawingType drawingType, EraserType eraserMode);
    virtual ~ButtonConfig();

public:
    void acceptActions(ToolHandler* toolHandler);
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
    string device;

    friend class Settings;
    friend class ButtonConfigGui;
};
