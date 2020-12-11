/*
 * Xournal++
 *
 * Handles Tools
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <util/Color.h>

#include "settings/Settings.h"
#include "settings/SettingsEnums.h"

#include "Tool.h"
#include "XournalType.h"

// enum for ptrs that are dynamically pointing to different tools
/**
 * @brief Enum for ptrs that are dynamically pointing to different tools
 *  - active: describes the currently active tool used for drawing
 *  - toolbar: describes the tool currently selected in the toolbar
 *
 * These tools are to be distinguished from ButtonTools which are mostly static
 * apart from changes to the Config
 *
 */
enum SelectedTool { active, toolbar };

class ToolListener {
public:
    /**
     * @brief Update the Cursor and the Toolbar based on the active color
     *
     */
    virtual void toolColorChanged() = 0;
    /**
     * @brief Change the color of the current selection based on the active Tool
     *
     */
    virtual void changeColorOfSelection() = 0;
    virtual void setCustomColorSelected() = 0;
    virtual void toolSizeChanged() = 0;
    virtual void toolFillChanged() = 0;
    virtual void toolLineStyleChanged() = 0;
    virtual void toolChanged() = 0;

    virtual ~ToolListener();
};

class ActionHandler;

class ToolHandler {
public:
    using ToolChangedCallback = std::function<void(ToolType)>;
    ToolHandler(ToolListener* stateChangedListener, ActionHandler* actionHandler, Settings* settings);
    virtual ~ToolHandler();

    /**
     * @brief Reset the Button tool with a new tooltype
     *
     * @param type Tooltype to be set for the button
     * @param button button which should be set
     */
    void resetButtonTool(ToolType type, Button button);

    /**
     * Select the color for the active tool and under certain circumstances toolbar selected tool
     *
     * If the current tool does not have the color capability but the toolbar selected tool has
     * the color can be set for the toolbar selected tool.
     * This is indicated by a little pen shown on top of the color in the UI.
     *
     * @param color Color
     * @param userSelection
     * 			true if the user selected the color
     * 			false if the color is selected by a tool change
     * 			and therefore should not be applied to a selection
     */
    void setColor(Color color, bool userSelection);

    /**
     * @brief Set the color for a Button
     * This is a separate function from `setColor` to prevent mixup of different usecases.
     *
     * @param color color to be set
     * @param button button to set color for
     */
    void setButtonColor(Color color, Button button);

    /**
     * @brief Get the Color of the active tool
     *
     * @return Color of active tool
     */
    Color getColor();

    /**
     * @brief Get the Fill of the active tool
     *
     * @return -1 if fill is disabled
     * @return int > 0 otherwise
     */
    int getFill();

    /**
     * @brief Get the Drawing Type one of selected tools
     *
     * @param selectedTool by the default the active one
     * @return DrawingType
     */
    DrawingType getDrawingType(SelectedTool selectedTool = SelectedTool::active);

    /**
     * @brief Set the Drawing Type of the toolbar selected tool
     * @note It is safer to always set the toolbar tool as the active tool could be pointing to a button tool which
     * could lead to hard to debug behaviour
     *
     * @param drawingType
     */
    void setDrawingType(DrawingType drawingType);

    /**
     * @brief Set the Button Drawing Type  of the button tool
     *
     * @param drawingType
     * @param button button tool to be selected
     */
    void setButtonDrawingType(DrawingType drawingType, Button button);

    /**
     * @brief Get the Line Style of active tool
     *
     * @return const LineStyle&
     */
    const LineStyle& getLineStyle();

    /**
     * @brief Get the Size of one of the selected tools
     *
     * @param selectedTool
     * @return ToolSize
     */
    ToolSize getSize(SelectedTool selectedTool = SelectedTool::active);

    /**
     * @brief Set the Size of toolbar selected tool
     * @note It is safer to always set the toolbar tool as the active tool could be pointing to a button tool which
     * could lead to hard to debug behaviour
     *
     * @param size is clamped to be within the defined range [0,5)
     */
    void setSize(ToolSize size);

    /**
     * @brief Set the Button Size
     *
     * @param size is clamped to be within the defined range [0,5)
     * @param button size will be applied to
     */
    void setButtonSize(ToolSize size, Button button);

    /**
     * @brief Get the Thickness of the active tool
     *
     * @return double
     */
    double getThickness();

    void setLineStyle(const LineStyle& style);

    ToolSize getPenSize();
    ToolSize getEraserSize();
    ToolSize getHighlighterSize();
    void setPenSize(ToolSize size);
    void setEraserSize(ToolSize size);
    void setHighlighterSize(ToolSize size);

    void setPenFillEnabled(bool fill, bool fireEvent = true);
    bool getPenFillEnabled();
    void setPenFill(int alpha);
    int getPenFill();

    void setHighlighterFillEnabled(bool fill, bool fireEvent = true);
    bool getHighlighterFillEnabled();
    void setHighlighterFill(int alpha);
    int getHighlighterFill();

    /**
     * @brief Set the toolbar selected tool to the type
     * This will also point the active tool to the same tool as the toolbar selected tool.
     * This ensure that the toolbar and the cursor are correctly updated right after selecting the tool in the toolbar.
     *
     * @param type
     */
    void selectTool(ToolType type);

    /**
     * @brief Get the Tool Type of active tool
     *
     * @return ToolType
     */
    ToolType getToolType();

    /**
     * @brief Update the Toolbar and the cursor based on the active Tool
     *
     */
    void fireToolChanged();

    /**
     * @brief Listen for tool changes.
     *
     * Different from the listener given to the constructor -- [listener]
     * here only listens for when the current tool is changed to another.
     *
     * @param listener A callback, called when the user/client
     *  changes tools.
     */
    void addToolChangedListener(ToolChangedCallback listener);

    /**
     * @brief Get the Tool of a certain type
     *
     * @param type
     * @return Tool&
     */
    Tool& getTool(ToolType type);

    /**
     * @brief Set the Eraser Type of the Eraser in the toolbar
     * @note Here the Eraser Tool in the toolbar is changed regardless of the the tool currently selected.
     * This is necessary to allow users to change the Eraser type for their Button Tools while having another tool
     * active. This is relevant in case of Eraser Type being set to "Don't Change" for the button.
     *
     * @param eraserType
     */
    void setEraserType(EraserType eraserType);

    /**
     * @brief Set the Button Eraser Type
     *
     * @param eraserType
     * @param button
     */
    void setButtonEraserType(EraserType eraserType, Button button);

    /**
     * @brief Get the Eraser Type
     * If the currently active Tool is an eraser it's type is returned (relevant for Buttontools).
     * If the currently active Tool is not a eraser the erasertype of the eraser in the toolbar is obtained.
     *
     * @param selectedTool
     * @return EraserType
     */
    EraserType getEraserType();

    /**
     * @brief Update the toolbar based on the Eraser type of the active tool
     *
     */
    void eraserTypeChanged();

    /**
     * @brief Check whether the selectedTool has a certain capability
     *
     * @param cap
     * @param selectedTool
     * @return true if tool has the capability
     * @return false if tool does not have the capability
     */
    bool hasCapability(ToolCapabilities cap, SelectedTool selectedTool = SelectedTool::active);

    /**
     * @brief Check whether the active tool is a Drawing tool.
     * Drawing tools are considered all tools that directly change the canvas.
     * Right now these are:
     *  - Highlighter
     *  - Pen
     *  - Eraser
     *
     * @return true if active tool is a drawing tool
     * @return false if active tool is no drawing tool
     */
    bool isDrawingTool();

    void saveSettings();
    void loadSettings();

    /**
     * @brief Point the active tool to the corresponding button tool if it is not already pointing to it
     *
     * @param button Button tool which should be pointed to
     * @return true if the active toolpointer was changed
     * @return false if the active toolpointer was not changed (it was already pointing to the right button)
     */
    bool pointActiveToolToButtonTool(Button button);
    /**
     * @brief Point the active tool to tool selected in the toolbar
     *
     * @return true if the active toolpointer was changed
     * @return false if the active toolpointer was not changed (it was already pointing to the toolbar-tool)
     */
    bool pointActiveToolToToolbarTool();

    [[maybe_unused]] std::array<std::unique_ptr<Tool>, TOOL_COUNT> const& getTools() const;

    /**
     * Change the selection tools capabilities, depending on the selected elements
     */
    void setSelectionEditTools(bool setColor, bool setSize, bool setFill);

    const double* getToolThickness(ToolType type);

    /**
     * Returns whether the current tool will create an element that may only reside on a single page even when the
     * pointer moves to another
     * @return
     */
    bool isSinglePageTool();

protected:
    void initTools();

private:
    std::array<std::unique_ptr<Tool>, TOOL_COUNT> tools;

    /**
     * @brief Get the Button Tool pointer based on enum
     *
     * @param button
     * @return Tool*
     */
    Tool* getButtonTool(Button button);

    /**
     * @brief Get the Selected Tool pointer based on enum
     *
     * @param selectedTool
     * @return Tool*
     */
    Tool* getSelectedTool(SelectedTool selectedTool);

    // active Tool which is used for drawing
    Tool* activeTool = nullptr;

    // tool which is selected in the toolbar
    Tool* toolbarSelectedTool = nullptr;

    // tools set for the different Buttons
    std::unique_ptr<Tool> stylusButton1Tool;
    std::unique_ptr<Tool> stylusButton2Tool;
    std::unique_ptr<Tool> eraserButtonTool;
    std::unique_ptr<Tool> mouseMiddleButtonTool;
    std::unique_ptr<Tool> mouseRightButtonTool;
    std::unique_ptr<Tool> touchDrawingButtonTool;

    std::vector<ToolChangedCallback> toolChangeListeners;

    ToolListener* stateChangeListener = nullptr;
    ActionHandler* actionHandler = nullptr;
    Settings* settings = nullptr;
};
