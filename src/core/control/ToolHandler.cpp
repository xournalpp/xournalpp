#include "ToolHandler.h"

#include <algorithm>  // for clamp
#include <cinttypes>  // for uint32_t
#include <cstdio>     // for size_t
#include <optional>   // for nullopt, optional
#include <string>     // for operator==, string, basic_string
#include <utility>    // for move

#include <glib.h>  // for g_warning, g_error

#include "control/Tool.h"               // for Tool, Tool::toolSizes
#include "control/settings/Settings.h"  // for SElement, Settings
#include "enums/ActionGroup.enum.h"     // for GROUP_ERASER_MODE
#include "enums/ActionType.enum.h"      // for ACTION_TOOL_ERASER_DELETE_STROKE
#include "model/StrokeStyle.h"          // for StrokeStyle
#include "util/Color.h"

#include "Actions.h"  // for ActionHandler

class LineStyle;


ToolListener::~ToolListener() = default;

ToolHandler::ToolHandler(ToolListener* stateChangeListener, ActionHandler* actionHandler, Settings* settings) {
    this->settings = settings;
    initTools();
    this->actionHandler = actionHandler;

    this->stateChangeListener = stateChangeListener;
}

void ToolHandler::initTools() {
    std::array<double, Tool::toolSizes> thickness;
    // pen thicknesses = 0.15, 0.3, 0.5, 0.8, 2 mm
    thickness[TOOL_SIZE_VERY_FINE] = 0.42;
    thickness[TOOL_SIZE_FINE] = 0.85;
    thickness[TOOL_SIZE_MEDIUM] = 1.41;
    thickness[TOOL_SIZE_THICK] = 2.26;
    thickness[TOOL_SIZE_VERY_THICK] = 5.67;
    tools[TOOL_PEN - TOOL_PEN] = std::make_unique<Tool>(
            "pen", TOOL_PEN, Colors::xopp_royalblue,
            TOOL_CAP_COLOR | TOOL_CAP_SIZE | TOOL_CAP_RULER | TOOL_CAP_RECTANGLE | TOOL_CAP_ELLIPSE | TOOL_CAP_ARROW |
                    TOOL_CAP_DOUBLE_ARROW | TOOL_CAP_SPLINE | TOOL_CAP_RECOGNIZER | TOOL_CAP_FILL | TOOL_CAP_DASH_LINE |
                    TOOL_CAP_LINE_STYLE,
            thickness);

    thickness[TOOL_SIZE_VERY_FINE] = 1;
    thickness[TOOL_SIZE_FINE] = 2.83;
    thickness[TOOL_SIZE_MEDIUM] = 8.50;
    thickness[TOOL_SIZE_THICK] = 12;
    thickness[TOOL_SIZE_VERY_THICK] = 18;
    tools[TOOL_ERASER - TOOL_PEN] =
            std::make_unique<Tool>("eraser", TOOL_ERASER, Colors::black, TOOL_CAP_SIZE, thickness);

    // highlighter thicknesses = 1, 3, 7 mm
    thickness[TOOL_SIZE_VERY_FINE] = 1;
    thickness[TOOL_SIZE_FINE] = 2.83;
    thickness[TOOL_SIZE_MEDIUM] = 8.50;
    thickness[TOOL_SIZE_THICK] = 19.84;
    thickness[TOOL_SIZE_VERY_THICK] = 30;
    tools[TOOL_HIGHLIGHTER - TOOL_PEN] = std::make_unique<Tool>(
            "highlighter", TOOL_HIGHLIGHTER, Colors::yellow,
            TOOL_CAP_COLOR | TOOL_CAP_SIZE | TOOL_CAP_RULER | TOOL_CAP_RECTANGLE | TOOL_CAP_ELLIPSE | TOOL_CAP_ARROW |
                    TOOL_CAP_DOUBLE_ARROW | TOOL_CAP_SPLINE | TOOL_CAP_RECOGNIZER | TOOL_CAP_FILL,
            thickness);

    tools[TOOL_TEXT - TOOL_PEN] =
            std::make_unique<Tool>("text", TOOL_TEXT, Colors::black, TOOL_CAP_COLOR, std::nullopt);

    tools[TOOL_IMAGE - TOOL_PEN] =
            std::make_unique<Tool>("image", TOOL_IMAGE, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_SELECT_RECT - TOOL_PEN] =
            std::make_unique<Tool>("selectRect", TOOL_SELECT_RECT, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_SELECT_REGION - TOOL_PEN] =
            std::make_unique<Tool>("selectRegion", TOOL_SELECT_REGION, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_SELECT_OBJECT - TOOL_PEN] =
            std::make_unique<Tool>("selectObject", TOOL_SELECT_OBJECT, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_VERTICAL_SPACE - TOOL_PEN] =
            std::make_unique<Tool>("verticalSpace", TOOL_VERTICAL_SPACE, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_HAND - TOOL_PEN] =
            std::make_unique<Tool>("hand", TOOL_HAND, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_PLAY_OBJECT - TOOL_PEN] =
            std::make_unique<Tool>("playObject", TOOL_PLAY_OBJECT, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_DRAW_RECT - TOOL_PEN] =
            std::make_unique<Tool>("drawRect", TOOL_DRAW_RECT, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_DRAW_ELLIPSE - TOOL_PEN] =
            std::make_unique<Tool>("drawEllipse", TOOL_DRAW_ELLIPSE, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_DRAW_ARROW - TOOL_PEN] =
            std::make_unique<Tool>("drawArrow", TOOL_DRAW_ARROW, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_DRAW_DOUBLE_ARROW - TOOL_PEN] = std::make_unique<Tool>("drawDoubleArrow", TOOL_DRAW_DOUBLE_ARROW,
                                                                      Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_DRAW_COORDINATE_SYSTEM - TOOL_PEN] = std::make_unique<Tool>(
            "drawCoordinateSystem", TOOL_DRAW_COORDINATE_SYSTEM, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_DRAW_SPLINE - TOOL_PEN] =
            std::make_unique<Tool>("drawSpline", TOOL_DRAW_SPLINE, Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_FLOATING_TOOLBOX - TOOL_PEN] = std::make_unique<Tool>("showFloatingToolbox", TOOL_FLOATING_TOOLBOX,
                                                                     Colors::black, TOOL_CAP_NONE, std::nullopt);

    tools[TOOL_SELECT_PDF_TEXT_LINEAR - TOOL_PEN] =
            std::make_unique<Tool>("selectPdfTextLinear", TOOL_SELECT_PDF_TEXT_LINEAR, Colors::black,
                                   TOOL_CAP_COLOR | TOOL_CAP_RULER, std::nullopt);

    tools[TOOL_SELECT_PDF_TEXT_RECT - TOOL_PEN] =
            std::make_unique<Tool>("selectPdfTextRect", TOOL_SELECT_PDF_TEXT_RECT, Colors::black,
                                   TOOL_CAP_COLOR | TOOL_CAP_RULER, std::nullopt);

    this->eraserButtonTool = std::make_unique<Tool>(*tools[TOOL_HIGHLIGHTER - TOOL_PEN]);
    this->stylusButton1Tool = std::make_unique<Tool>(*tools[TOOL_HIGHLIGHTER - TOOL_PEN]);
    this->stylusButton2Tool = std::make_unique<Tool>(*tools[TOOL_HIGHLIGHTER - TOOL_PEN]);
    this->mouseMiddleButtonTool = std::make_unique<Tool>(*tools[TOOL_HIGHLIGHTER - TOOL_PEN]);
    this->mouseRightButtonTool = std::make_unique<Tool>(*tools[TOOL_HIGHLIGHTER - TOOL_PEN]);
    this->touchDrawingButtonTool = std::make_unique<Tool>(*tools[TOOL_HIGHLIGHTER - TOOL_PEN]);

    this->toolbarSelectedTool = &getTool(TOOL_PEN);
    this->activeTool = &getTool(TOOL_PEN);
}

ToolHandler::~ToolHandler() {
    // Do not delete settings!
    this->settings = nullptr;
}

void ToolHandler::setEraserType(EraserType eraserType) {
    Tool& tool = this->getTool(TOOL_ERASER);
    tool.setEraserType(eraserType);
    eraserTypeChanged();
}

void ToolHandler::setButtonEraserType(EraserType eraserType, Button button) {
    Tool* tool = getButtonTool(button);
    tool->setEraserType(eraserType);
    eraserTypeChanged();
}

void ToolHandler::eraserTypeChanged() {
    if (this->actionHandler == nullptr) {
        return;
    }

    switch (this->getEraserType()) {
        case ERASER_TYPE_DELETE_STROKE:
            this->actionHandler->fireActionSelected(GROUP_ERASER_MODE, ACTION_TOOL_ERASER_DELETE_STROKE);
            break;

        case ERASER_TYPE_WHITEOUT:
            this->actionHandler->fireActionSelected(GROUP_ERASER_MODE, ACTION_TOOL_ERASER_WHITEOUT);
            break;

        case ERASER_TYPE_DEFAULT:
        default:
            this->actionHandler->fireActionSelected(GROUP_ERASER_MODE, ACTION_TOOL_ERASER_STANDARD);
            break;
    }
}

auto ToolHandler::getEraserType() const -> EraserType {
    // if active tool is eraser get its type
    if (this->activeTool->type == TOOL_ERASER)
        return this->activeTool->getEraserType();
    else  // otherwise get EraserType of Toolbar Eraser
        return this->getTool(TOOL_ERASER).getEraserType();
}

void ToolHandler::selectTool(ToolType type) {
    if (type < 1 || type > TOOL_COUNT) {
        g_warning("unknown tool selected: %i\n", type);
        return;
    }
    this->toolbarSelectedTool = &getTool(type);
    // set activeTool is necessary for fireToolChanged()
    // if called after this method
    // to result in the correct Button shown as active
    this->activeTool = this->toolbarSelectedTool;
}

void ToolHandler::fireToolChanged() const {
    for (auto&& listener: this->toolChangeListeners) { listener(this->activeTool->type); }

    stateChangeListener->toolChanged();
}

void ToolHandler::addToolChangedListener(ToolChangedCallback listener) {
    toolChangeListeners.emplace_back(std::move(listener));
}

auto ToolHandler::getTool(ToolType type) const -> Tool& { return *(this->tools[type - TOOL_PEN]); }

auto ToolHandler::getActiveTool() const -> Tool* {return this->activeTool; }

auto ToolHandler::getToolType() const -> ToolType {
    Tool* tool = this->activeTool;
    return tool->type;
}

auto ToolHandler::hasCapability(ToolCapabilities cap, SelectedTool selectedTool) const -> bool {
    Tool* tool = getSelectedTool(selectedTool);
    return (tool->capabilities & cap) != 0;
}

auto ToolHandler::isDrawingTool() const -> bool {
    Tool* tool = this->activeTool;
    return tool->isDrawingTool();
}

auto ToolHandler::getSize(SelectedTool selectedTool) const -> ToolSize {
    Tool* tool = getSelectedTool(selectedTool);
    return tool->getSize();
}

auto ToolHandler::getPenSize() const -> ToolSize { return tools[TOOL_PEN - TOOL_PEN]->getSize(); }

auto ToolHandler::getEraserSize() const -> ToolSize { return tools[TOOL_ERASER - TOOL_PEN]->getSize(); }

auto ToolHandler::getHighlighterSize() const -> ToolSize { return tools[TOOL_HIGHLIGHTER - TOOL_PEN]->getSize(); }

void ToolHandler::setPenSize(ToolSize size) {
    this->tools[TOOL_PEN - TOOL_PEN]->setSize(size);

    if (this->activeTool->type == TOOL_PEN) {
        this->stateChangeListener->toolSizeChanged();
    }
}

void ToolHandler::setEraserSize(ToolSize size) {
    this->tools[TOOL_ERASER - TOOL_PEN]->setSize(size);

    if (this->activeTool->type == TOOL_ERASER) {
        this->stateChangeListener->toolSizeChanged();
    }
}

void ToolHandler::setHighlighterSize(ToolSize size) {
    this->tools[TOOL_HIGHLIGHTER - TOOL_PEN]->setSize(size);

    if (this->activeTool->type == TOOL_HIGHLIGHTER) {
        this->stateChangeListener->toolSizeChanged();
    }
}

void ToolHandler::setPenFillEnabled(bool fill, bool fireEvent) {
    this->tools[TOOL_PEN - TOOL_PEN]->setFill(fill);

    if (this->activeTool->type == TOOL_PEN && fireEvent) {
        this->stateChangeListener->toolFillChanged();
    }
}

auto ToolHandler::getPenFillEnabled() const -> bool { return this->tools[TOOL_PEN - TOOL_PEN]->getFill(); }

void ToolHandler::setPenFill(int alpha) { this->tools[TOOL_PEN - TOOL_PEN]->setFillAlpha(alpha); }

auto ToolHandler::getPenFill() const -> int { return this->tools[TOOL_PEN - TOOL_PEN]->getFillAlpha(); }

void ToolHandler::setHighlighterFillEnabled(bool fill, bool fireEvent) {
    this->tools[TOOL_HIGHLIGHTER - TOOL_PEN]->setFill(fill);

    if (this->activeTool->type == TOOL_HIGHLIGHTER && fireEvent) {
        this->stateChangeListener->toolFillChanged();
    }
}

auto ToolHandler::getHighlighterFillEnabled() const -> bool {
    return this->tools[TOOL_HIGHLIGHTER - TOOL_PEN]->getFill();
}

void ToolHandler::setHighlighterFill(int alpha) { this->tools[TOOL_HIGHLIGHTER - TOOL_PEN]->setFillAlpha(alpha); }

auto ToolHandler::getHighlighterFill() const -> int { return this->tools[TOOL_HIGHLIGHTER - TOOL_PEN]->getFillAlpha(); }

auto ToolHandler::getThickness() const -> double {
    Tool* tool = this->activeTool;
    if (tool->thickness) {
        return tool->thickness.value()[tool->getSize()];
    }

    g_warning("Request size of \"%s\"", tool->getName().c_str());
    return 0;
}

void ToolHandler::setSize(ToolSize size) {
    ToolSize clippedSize = std::clamp(size, TOOL_SIZE_VERY_FINE, TOOL_SIZE_VERY_THICK);
    if (clippedSize != size)
        g_warning("ToolHandler::setSize: Invalid size! %i", size);

    Tool* tool = this->toolbarSelectedTool;
    tool->setSize(clippedSize);
    this->stateChangeListener->toolSizeChanged();
}

void ToolHandler::setButtonSize(ToolSize size, Button button) {
    ToolSize clippedSize = std::clamp(size, TOOL_SIZE_VERY_FINE, TOOL_SIZE_VERY_THICK);
    if (clippedSize != size)
        g_warning("ToolHandler::setSize: Invalid size! %i", size);

    Tool* tool = getButtonTool(button);
    tool->setSize(clippedSize);
    this->stateChangeListener->toolSizeChanged();
}

void ToolHandler::setLineStyle(const LineStyle& style) {
    this->tools[TOOL_PEN - TOOL_PEN]->setLineStyle(style);
    this->stateChangeListener->toolLineStyleChanged();
}

void ToolHandler::setColor(Color color, bool userSelection) {
    if (this->activeTool != this->toolbarSelectedTool && !this->hasCapability(TOOL_CAP_COLOR, SelectedTool::active)) {
        this->toolbarSelectedTool->setColor(color);
    }
    Tool* tool = this->activeTool;
    tool->setColor(color);
    this->stateChangeListener->toolColorChanged();
    if (userSelection)
        this->stateChangeListener->changeColorOfSelection();
    this->stateChangeListener->setCustomColorSelected();
}

void ToolHandler::setButtonColor(Color color, Button button) {
    Tool* tool = this->getButtonTool(button);
    tool->setColor(color);
    this->stateChangeListener->toolColorChanged();
    this->stateChangeListener->setCustomColorSelected();
}

auto ToolHandler::getColor() const -> Color {
    Tool* tool = this->activeTool;
    return tool->getColor();
}

/**
 * @return -1 if fill is disabled, else the fill alpha value
 */
auto ToolHandler::getFill() const -> int {
    Tool* tool = this->activeTool;
    if (!tool->getFill()) {
        return -1;
    }
    return tool->getFillAlpha();
}

auto ToolHandler::getLineStyle() const -> const LineStyle& {
    Tool* tool = this->activeTool;
    return tool->getLineStyle();
}

auto ToolHandler::getDrawingType(SelectedTool selectedTool) const -> DrawingType {
    Tool* tool = getSelectedTool(selectedTool);
    return tool->getDrawingType();
}

void ToolHandler::setDrawingType(DrawingType drawingType) {
    Tool* tool = this->toolbarSelectedTool;
    tool->setDrawingType(drawingType);
}

void ToolHandler::setButtonDrawingType(DrawingType drawingType, Button button) {
    Tool* tool = getButtonTool(button);
    tool->setDrawingType(drawingType);
}

auto ToolHandler::getTools() const -> std::array<std::unique_ptr<Tool>, TOOL_COUNT> const& { return tools; }

void ToolHandler::saveSettings() const {
    SElement& s = settings->getCustomElement("tools");
    s.clear();

    s.setString("current", this->activeTool->getName());

    for (auto&& tool: tools) {
        SElement& st = s.child(tool->getName());
        if (tool->hasCapability(TOOL_CAP_COLOR)) {
            st.setIntHex("color", int(uint32_t(tool->getColor())));
        }

        st.setString("drawingType", drawingTypeToString(tool->getDrawingType()));

        if (tool->hasCapability(TOOL_CAP_SIZE)) {
            std::string value;
            switch (tool->getSize()) {
                case TOOL_SIZE_VERY_FINE:
                    value = "VERY_FINE";
                    break;
                case TOOL_SIZE_FINE:
                    value = "THIN";
                    break;
                case TOOL_SIZE_MEDIUM:
                    value = "MEDIUM";
                    break;
                case TOOL_SIZE_THICK:
                    value = "BIG";
                    break;
                case TOOL_SIZE_VERY_THICK:
                    value = "VERY_BIG";
                    break;
                default:
                    value = "";
            }

            st.setString("size", value);
        }

        if (tool->type == TOOL_PEN || tool->type == TOOL_HIGHLIGHTER) {
            st.setInt("fill", tool->getFill());
            st.setInt("fillAlpha", tool->getFillAlpha());
        }

        if (tool->type == TOOL_PEN) {
            st.setString("style", StrokeStyle::formatStyle(tool->getLineStyle()));
        }

        if (tool->type == TOOL_ERASER) {
            if (this->getEraserType() == ERASER_TYPE_DELETE_STROKE) {
                st.setString("type", "deleteStroke");
            } else if (this->getEraserType() == ERASER_TYPE_WHITEOUT) {
                st.setString("type", "whiteout");
            } else  // ERASER_TYPE_DEFAULT
            {
                st.setString("type", "default");
            }
        }
    }

    settings->customSettingsChanged();
}

void ToolHandler::loadSettings() {
    SElement& s = settings->getCustomElement("tools");

    std::string selectedTool;
    if (s.getString("current", selectedTool)) {
        for (auto&& tool: tools) {
            SElement& st = s.child(tool->getName());

            if (selectedTool == tool->getName()) {
                this->activeTool = tool.get();
            }

            int iColor{};
            if (tool->hasCapability(TOOL_CAP_COLOR) && st.getInt("color", iColor)) {
                tool->setColor(Color(iColor));
            }

            std::string drawingType;
            if (st.getString("drawingType", drawingType)) {
                tool->setDrawingType(drawingTypeFromString(drawingType));
            }

            int fill = -1;
            if (st.getInt("fill", fill)) {
                tool->setFill(fill);
            }
            int fillAlpha = -1;
            if (st.getInt("fillAlpha", fillAlpha)) {
                tool->setFillAlpha(fillAlpha);
            }

            std::string style;
            if (st.getString("style", style)) {
                tool->setLineStyle(StrokeStyle::parseStyle(style.c_str()));
            }

            std::string value;
            if (tool->hasCapability(TOOL_CAP_SIZE) && st.getString("size", value)) {
                if (value == "VERY_FINE") {
                    tool->setSize(TOOL_SIZE_VERY_FINE);
                } else if (value == "THIN") {
                    tool->setSize(TOOL_SIZE_FINE);
                } else if (value == "MEDIUM") {
                    tool->setSize(TOOL_SIZE_MEDIUM);
                } else if (value == "BIG") {
                    tool->setSize(TOOL_SIZE_THICK);
                } else if (value == "VERY_BIG") {
                    tool->setSize(TOOL_SIZE_VERY_THICK);
                } else {
                    g_warning("Settings::Unknown tool size: %s\n", value.c_str());
                }
            }

            if (tool->type == TOOL_ERASER) {
                std::string type;

                if (st.getString("type", type)) {
                    if (type == "deleteStroke") {
                        setEraserType(ERASER_TYPE_DELETE_STROKE);
                    } else if (type == "whiteout") {
                        setEraserType(ERASER_TYPE_WHITEOUT);
                    } else {
                        setEraserType(ERASER_TYPE_DEFAULT);
                    }
                    eraserTypeChanged();
                }
            }
        }
    }
}

bool ToolHandler::pointActiveToolToButtonTool(Button button) {
    Tool* tool = getButtonTool(button);
    if (this->activeTool == tool)
        return false;
    this->activeTool = tool;
    return true;
}

bool ToolHandler::pointActiveToolToToolbarTool() {
    if (this->activeTool == this->toolbarSelectedTool)
        return false;
    this->activeTool = this->toolbarSelectedTool;
    return true;
}

auto ToolHandler::getToolThickness(ToolType type) const -> const double* {
    return this->tools[type - TOOL_PEN]->thickness.value().data();
}

/**
 * Change the selection tools capabilities, depending on the selected elements
 */
void ToolHandler::setSelectionEditTools(bool setColor, bool setSize, bool setFill, bool setLineStyle) {
    // For all selection tools, apply the features
    for (size_t i = TOOL_SELECT_RECT - TOOL_PEN; i <= TOOL_SELECT_OBJECT - TOOL_PEN; i++) {
        Tool* t = tools[i].get();
        t->setCapability(TOOL_CAP_COLOR, setColor);
        t->setCapability(TOOL_CAP_SIZE, setSize);
        t->setCapability(TOOL_CAP_FILL, setFill);
        t->setCapability(TOOL_CAP_LINE_STYLE, setLineStyle);
        t->setSize(TOOL_SIZE_NONE);
        t->setColor(Color(-1));
        t->setFill(false);
    }

    if (this->activeTool->type == TOOL_SELECT_RECT || this->activeTool->type == TOOL_SELECT_REGION ||
        this->activeTool->type == TOOL_SELECT_OBJECT || this->activeTool->type == TOOL_PLAY_OBJECT) {
        this->stateChangeListener->toolColorChanged();
        this->stateChangeListener->toolSizeChanged();
        this->stateChangeListener->toolFillChanged();
        this->stateChangeListener->toolLineStyleChanged();
        this->fireToolChanged();
    }
}

auto ToolHandler::isSinglePageTool() const -> bool {
    ToolType toolType = this->getToolType();
    DrawingType drawingType = this->getDrawingType();

    return ((toolType == TOOL_PEN || toolType == TOOL_HIGHLIGHTER) &&
            (drawingType == DRAWING_TYPE_ARROW || drawingType == DRAWING_TYPE_DOUBLE_ARROW ||
             drawingType == DRAWING_TYPE_ELLIPSE || drawingType == DRAWING_TYPE_COORDINATE_SYSTEM ||
             drawingType == DRAWING_TYPE_LINE || drawingType == DRAWING_TYPE_RECTANGLE ||
             drawingType == DRAWING_TYPE_SPLINE)) ||
           toolType == TOOL_SELECT_REGION || toolType == TOOL_SELECT_RECT || toolType == TOOL_SELECT_OBJECT ||
           toolType == TOOL_DRAW_RECT || toolType == TOOL_DRAW_ELLIPSE || toolType == TOOL_DRAW_COORDINATE_SYSTEM ||
           toolType == TOOL_DRAW_ARROW || toolType == TOOL_DRAW_DOUBLE_ARROW || toolType == TOOL_FLOATING_TOOLBOX ||
           toolType == TOOL_DRAW_SPLINE || toolType == TOOL_SELECT_PDF_TEXT_LINEAR ||
           toolType == TOOL_SELECT_PDF_TEXT_RECT;
}

auto ToolHandler::getSelectedTool(SelectedTool selectedTool) const -> Tool* {
    switch (selectedTool) {
        case SelectedTool::active:
            return this->activeTool;
        case SelectedTool::toolbar:
            return this->toolbarSelectedTool;
        default:
            g_error("This button does not exist.");
    }
}

auto ToolHandler::getButtonTool(Button button) const -> Tool* {
    switch (button) {
        case Button::BUTTON_ERASER:
            return this->eraserButtonTool.get();
        case Button::BUTTON_STYLUS_ONE:
            return this->stylusButton1Tool.get();
        case Button::BUTTON_STYLUS_TWO:
            return this->stylusButton2Tool.get();
        case Button::BUTTON_MOUSE_MIDDLE:
            return this->mouseMiddleButtonTool.get();
        case Button::BUTTON_MOUSE_RIGHT:
            return this->mouseRightButtonTool.get();
        case Button::BUTTON_TOUCH:
            return this->touchDrawingButtonTool.get();
        default:
            g_error("This button is not defined for ToolHandler.");
    }
}

void ToolHandler::resetButtonTool(ToolType type, Button button) {
    auto& tool = *(tools[type - TOOL_PEN]);
    switch (button) {
        case Button::BUTTON_ERASER:
            this->eraserButtonTool.reset(new Tool(tool));
            break;
        case Button::BUTTON_STYLUS_ONE:
            this->stylusButton1Tool.reset(new Tool(tool));
            break;
        case Button::BUTTON_STYLUS_TWO:
            this->stylusButton2Tool.reset(new Tool(tool));
            break;
        case Button::BUTTON_MOUSE_MIDDLE:
            this->mouseMiddleButtonTool.reset(new Tool(tool));
            break;
        case Button::BUTTON_MOUSE_RIGHT:
            this->mouseRightButtonTool.reset(new Tool(tool));
            break;
        case Button::BUTTON_TOUCH:
            this->touchDrawingButtonTool.reset(new Tool(tool));
            break;
        default:
            g_error("This button is not defined for ToolHandler.");
    }
}
