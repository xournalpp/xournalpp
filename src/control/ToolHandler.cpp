#include "Actions.h"
#include "LastSelectedTool.h"
#include "ToolHandler.h"
#include "model/StrokeStyle.h"
#include <Util.h>

#include <gtk/gtk.h>
#include <cstdio>
#include <config-debug.h>

ToolListener::~ToolListener() = default;

ToolHandler::ToolHandler(ToolListener* listener, ActionHandler* actionHandler, Settings* settings)
{
	this->settings = settings;
	initTools();
	this->listener = listener;
	this->actionHandler = actionHandler;
}

void ToolHandler::initTools()
{
	auto* thickness = new double[5];
	// pen thicknesses = 0.15, 0.3, 0.5, 0.8, 2 mm
	thickness[TOOL_SIZE_VERY_FINE] = 0.42;
	thickness[TOOL_SIZE_FINE] = 0.85;
	thickness[TOOL_SIZE_MEDIUM] = 1.41;
	thickness[TOOL_SIZE_THICK] = 2.26;
	thickness[TOOL_SIZE_VERY_THICK] = 5.67;
	tools[TOOL_PEN - TOOL_PEN] = std::make_unique<Tool>(
	        "pen", TOOL_PEN, 0x3333CC,
	        TOOL_CAP_COLOR | TOOL_CAP_SIZE | TOOL_CAP_RULER | TOOL_CAP_RECTANGLE | TOOL_CAP_CIRCLE | TOOL_CAP_ARROW |
	                TOOL_CAP_RECOGNIZER | TOOL_CAP_FILL | TOOL_CAP_DASH_LINE,
	        thickness);

	thickness = new double[5];
	thickness[TOOL_SIZE_VERY_FINE] = 1;
	thickness[TOOL_SIZE_FINE] = 2.83;
	thickness[TOOL_SIZE_MEDIUM] = 8.50;
	thickness[TOOL_SIZE_THICK] = 12;
	thickness[TOOL_SIZE_VERY_THICK] = 18;
	tools[TOOL_ERASER - TOOL_PEN] = std::make_unique<Tool>("eraser", TOOL_ERASER, 0x000000, TOOL_CAP_SIZE, thickness);

	// highlighter thicknesses = 1, 3, 7 mm
	thickness = new double[5];
	thickness[TOOL_SIZE_VERY_FINE] = 1;
	thickness[TOOL_SIZE_FINE] = 2.83;
	thickness[TOOL_SIZE_MEDIUM] = 8.50;
	thickness[TOOL_SIZE_THICK] = 19.84;
	thickness[TOOL_SIZE_VERY_THICK] = 30;
	tools[TOOL_HILIGHTER - TOOL_PEN] =
	        std::make_unique<Tool>("hilighter", TOOL_HILIGHTER, 0xFFFF00,
	                               TOOL_CAP_COLOR | TOOL_CAP_SIZE | TOOL_CAP_RULER | TOOL_CAP_RECTANGLE |
	                                       TOOL_CAP_CIRCLE | TOOL_CAP_ARROW | TOOL_CAP_RECOGNIZER | TOOL_CAP_FILL,
	                               thickness);

	tools[TOOL_TEXT - TOOL_PEN] = std::make_unique<Tool>("text", TOOL_TEXT, 0x000000, TOOL_CAP_COLOR, nullptr);

	tools[TOOL_IMAGE - TOOL_PEN] = std::make_unique<Tool>("image", TOOL_IMAGE, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_SELECT_RECT - TOOL_PEN] =
	        std::make_unique<Tool>("selectRect", TOOL_SELECT_RECT, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_SELECT_REGION - TOOL_PEN] =
	        std::make_unique<Tool>("selectRegion", TOOL_SELECT_REGION, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_SELECT_OBJECT - TOOL_PEN] =
	        std::make_unique<Tool>("selectObject", TOOL_SELECT_OBJECT, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_VERTICAL_SPACE - TOOL_PEN] =
	        std::make_unique<Tool>("verticalSpace", TOOL_VERTICAL_SPACE, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_HAND - TOOL_PEN] = std::make_unique<Tool>("hand", TOOL_HAND, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_PLAY_OBJECT - TOOL_PEN] =
	        std::make_unique<Tool>("playObject", TOOL_PLAY_OBJECT, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_DRAW_RECT - TOOL_PEN] =
	        std::make_unique<Tool>("drawRect", TOOL_DRAW_RECT, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_DRAW_CIRCLE - TOOL_PEN] =
	        std::make_unique<Tool>("drawCircle", TOOL_DRAW_CIRCLE, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_DRAW_ARROW - TOOL_PEN] =
	        std::make_unique<Tool>("drawArrow", TOOL_DRAW_ARROW, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_DRAW_COORDINATE_SYSTEM - TOOL_PEN] = std::make_unique<Tool>(
	        "drawCoordinateSystem", TOOL_DRAW_COORDINATE_SYSTEM, 0x000000, TOOL_CAP_NONE, nullptr);

	tools[TOOL_FLOATING_TOOLBOX - TOOL_PEN] =
	        std::make_unique<Tool>("showFloatingToolbox", TOOL_FLOATING_TOOLBOX, 0x000000, TOOL_CAP_NONE, nullptr);


	selectTool(TOOL_PEN);
}

ToolHandler::~ToolHandler()
{
	// Do not delete settings!
	this->settings = nullptr;
}

void ToolHandler::setEraserType(EraserType eraserType)
{
	this->eraserType = eraserType;
	eraserTypeChanged();
}

void ToolHandler::eraserTypeChanged()
{
	if (this->actionHandler == nullptr)
	{
		return;
	}

	switch (this->eraserType)
	{
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

auto ToolHandler::getEraserType() -> EraserType
{
	return this->eraserType;
}

void ToolHandler::selectTool(ToolType type, bool fireToolChanged)
{
	if (type < 1 || type > TOOL_COUNT)
	{
		g_warning("unknown tool selected: %i\n", type);
		return;
	}
	this->current = tools[type - TOOL_PEN].get();

	if (fireToolChanged)
	{
		this->fireToolChanged();
	}
}

void ToolHandler::fireToolChanged()
{
	if (listener)
	{
		listener->toolChanged();
	}
}

auto ToolHandler::getTool(ToolType type) -> Tool&
{
	return *(this->tools[type - TOOL_PEN]);
}

auto ToolHandler::getToolType() -> ToolType
{
	return this->current->type;
}

auto ToolHandler::hasCapability(ToolCapabilities cap) -> bool
{
	return (current->capabilities & cap) != 0;
}

auto ToolHandler::getSize() -> ToolSize
{
	return this->current->getSize();
}

auto ToolHandler::getPenSize() -> ToolSize
{
	return tools[TOOL_PEN - TOOL_PEN]->getSize();
}

auto ToolHandler::getEraserSize() -> ToolSize
{
	return tools[TOOL_ERASER - TOOL_PEN]->getSize();
}

auto ToolHandler::getHilighterSize() -> ToolSize
{
	return tools[TOOL_HILIGHTER - TOOL_PEN]->getSize();
}

void ToolHandler::setPenSize(ToolSize size)
{
	this->tools[TOOL_PEN - TOOL_PEN]->setSize(size);

	if (this->current->type == TOOL_PEN)
	{
		this->listener->toolSizeChanged();
	}
}

void ToolHandler::setEraserSize(ToolSize size)
{
	this->tools[TOOL_ERASER - TOOL_PEN]->setSize(size);

	if (this->current->type == TOOL_ERASER)
	{
		this->listener->toolSizeChanged();
	}
}

void ToolHandler::setHilighterSize(ToolSize size)
{
	this->tools[TOOL_HILIGHTER - TOOL_PEN]->setSize(size);

	if (this->current->type == TOOL_HILIGHTER)
	{
		this->listener->toolSizeChanged();
	}
}

void ToolHandler::setPenFillEnabled(bool fill, bool fireEvent)
{
	this->tools[TOOL_PEN - TOOL_PEN]->setFill(fill);

	if (this->current->type == TOOL_PEN && fireEvent)
	{
		this->listener->toolFillChanged();
	}
}

auto ToolHandler::getPenFillEnabled() -> bool
{
	return this->tools[TOOL_PEN - TOOL_PEN]->getFill();
}

void ToolHandler::setPenFill(int alpha)
{
	this->tools[TOOL_PEN - TOOL_PEN]->setFillAlpha(alpha);
}

auto ToolHandler::getPenFill() -> int
{
	return this->tools[TOOL_PEN - TOOL_PEN]->getFillAlpha();
}

void ToolHandler::setHilighterFillEnabled(bool fill, bool fireEvent)
{
	this->tools[TOOL_HILIGHTER - TOOL_PEN]->setFill(fill);

	if (this->current->type == TOOL_HILIGHTER && fireEvent)
	{
		this->listener->toolFillChanged();
	}
}

auto ToolHandler::getHilighterFillEnabled() -> bool
{
	return this->tools[TOOL_HILIGHTER - TOOL_PEN]->getFill();
}

void ToolHandler::setHilighterFill(int alpha)
{
	this->tools[TOOL_HILIGHTER - TOOL_PEN]->setFillAlpha(alpha);
}

auto ToolHandler::getHilighterFill() -> int
{
	return this->tools[TOOL_HILIGHTER - TOOL_PEN]->getFillAlpha();
}

auto ToolHandler::getThickness() -> double
{
	if (this->current->thickness)
	{
		return this->current->thickness[this->current->getSize()];
	}


	g_warning("Request size of \"%s\"", this->current->getName().c_str());
	return 0;
}

void ToolHandler::setSize(ToolSize size)
{
	if (size < TOOL_SIZE_VERY_FINE || size > TOOL_SIZE_VERY_THICK)
	{
		g_warning("ToolHandler::setSize: Invalid size! %i", size);
		return;
	}

	this->current->setSize(size);
	this->listener->toolSizeChanged();
}

void ToolHandler::setLineStyle(const LineStyle& style)
{
	this->tools[TOOL_PEN - TOOL_PEN]->setLineStyle(style);
	this->listener->toolLineStyleChanged();
}

/**
 * Select the color for the tool
 *
 * @param color Color
 * @param userSelection
 * 			true if the user selected the color
 * 			false if the color is selected by a tool change
 * 			and therefore should not be applied to a selection
 */
void ToolHandler::setColor(int color, bool userSelection)
{
	this->colorFound = false;

	this->current->setColor(color);
	this->listener->toolColorChanged(userSelection);

	if (!colorFound)
	{
		this->listener->setCustomColorSelected();
	}
}

auto ToolHandler::getColor() -> int
{
	return current->getColor();
}

/**
 * @return -1 if fill is disabled, else the fill alpha value
 */
auto ToolHandler::getFill() -> int
{
	if (!current->getFill())
	{
		return -1;
	}

	return current->getFillAlpha();
}

auto ToolHandler::getLineStyle() -> const LineStyle&
{
	return current->getLineStyle();
}

auto ToolHandler::getDrawingType() -> DrawingType
{
	return current->getDrawingType();
}

void ToolHandler::setDrawingType(DrawingType drawingType)
{
	current->setDrawingType(drawingType);
}

void ToolHandler::setColorFound()
{
	this->colorFound = true;
}

auto ToolHandler::getTools() const -> std::array<std::unique_ptr<Tool>, TOOL_COUNT> const&
{
	return tools;
}

void ToolHandler::saveSettings()
{
	SElement& s = settings->getCustomElement("tools");
	s.clear();

	s.setString("current", this->current->getName());

	for (auto&& tool: tools)
	{
		SElement& st = s.child(tool->getName());
		if (tool->hasCapability(TOOL_CAP_COLOR))
		{
			st.setIntHex("color", tool->getColor());
		}

		st.setString("drawingType", drawingTypeToString(tool->getDrawingType()));

		if (tool->hasCapability(TOOL_CAP_SIZE))
		{
			string value;
			switch (tool->getSize())
			{
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

		if (tool->type == TOOL_PEN || tool->type == TOOL_HILIGHTER)
		{
			st.setInt("fill", tool->getFill());
			st.setInt("fillAlpha", tool->getFillAlpha());
		}

		if (tool->type == TOOL_PEN)
		{
			st.setString("style", StrokeStyle::formatStyle(tool->getLineStyle()));
		}

		if (tool->type == TOOL_ERASER)
		{
			if (this->eraserType == ERASER_TYPE_DELETE_STROKE)
			{
				st.setString("type", "deleteStroke");
			}
			else if (this->eraserType == ERASER_TYPE_WHITEOUT)
			{
				st.setString("type", "whiteout");
			}
			else //ERASER_TYPE_DEFAULT
			{
				st.setString("type", "default");
			}
		}
	}

	settings->customSettingsChanged();
}

void ToolHandler::loadSettings()
{
	SElement& s = settings->getCustomElement("tools");

	string selectedTool;
	if (s.getString("current", selectedTool))
	{
		for (auto&& tool: tools)
		{
			SElement& st = s.child(tool->getName());

			if (selectedTool == tool->getName())
			{
				this->current = tool.get();
			}

			int color = 0;
			if (tool->hasCapability(TOOL_CAP_COLOR) && st.getInt("color", color))
			{
				tool->setColor(color);
			}

			string drawingType;
			if (st.getString("drawingType", drawingType))
			{
				tool->setDrawingType(drawingTypeFromString(drawingType));
			}

			int fill = -1;
			if (st.getInt("fill", fill))
			{
				tool->setFill(fill);
			}
			int fillAlpha = -1;
			if (st.getInt("fillAlpha", fillAlpha))
			{
				tool->setFillAlpha(fillAlpha);
			}

			string style;
			if (st.getString("style", style))
			{
				tool->setLineStyle(StrokeStyle::parseStyle(style.c_str()));
			}

			string value;
			if (tool->hasCapability(TOOL_CAP_SIZE) && st.getString("size", value))
			{
				if (value == "VERY_FINE")
				{
					tool->setSize(TOOL_SIZE_VERY_FINE);
				}
				else if (value == "THIN")
				{
					tool->setSize(TOOL_SIZE_FINE);
				}
				else if (value == "MEDIUM")
				{
					tool->setSize(TOOL_SIZE_MEDIUM);
				}
				else if (value == "BIG")
				{
					tool->setSize(TOOL_SIZE_THICK);
				}
				else if (value == "VERY_BIG")
				{
					tool->setSize(TOOL_SIZE_VERY_THICK);
				}
				else
				{
					g_warning("Settings::Unknown tool size: %s\n", value.c_str());
				}
			}

			if (tool->type == TOOL_ERASER)
			{
				string type;

				if (st.getString("type", type))
				{
					if (type == "deleteStroke")
					{
						setEraserType(ERASER_TYPE_DELETE_STROKE);
					}
					else if (type == "whiteout")
					{
						setEraserType(ERASER_TYPE_WHITEOUT);
					}
					else
					{
						setEraserType(ERASER_TYPE_DEFAULT);
					}
					eraserTypeChanged();
				}
			}
		}
	}
}

void ToolHandler::copyCurrentConfig()
{
	// If there is no last config, create one, if there is already one
	// do not overwrite this config!
	if (this->lastSelectedTool == nullptr)
	{
		this->lastSelectedTool = new LastSelectedTool(this->current);
	}
}

void ToolHandler::restoreLastConfig()
{
	if (this->lastSelectedTool == nullptr)
	{
		return;
	}

	this->current = this->lastSelectedTool->restoreAndGet();
	delete this->lastSelectedTool;
	this->lastSelectedTool = nullptr;

	this->listener->toolColorChanged(false);
	this->listener->toolSizeChanged();
	this->fireToolChanged();
}

auto ToolHandler::getToolThickness(ToolType type) -> const double*
{
	return this->tools[type - TOOL_PEN]->thickness;
}

/**
 * Change the selection tools capabilities, depending on the selected elements
 */
void ToolHandler::setSelectionEditTools(bool setColor, bool setSize, bool setFill)
{
	// For all selection tools, apply the features
	for (size_t i = TOOL_SELECT_RECT - TOOL_PEN; i <= TOOL_SELECT_OBJECT - TOOL_PEN; i++)
	{
		Tool* t = tools[i].get();
		t->setCapability(TOOL_CAP_COLOR, setColor);
		t->setCapability(TOOL_CAP_SIZE, setSize);
		t->setCapability(TOOL_CAP_FILL, setFill);
		t->setSize(TOOL_SIZE_NONE);
		t->setColor(-1);
		t->setFill(false);
	}

	if (this->current->type == TOOL_SELECT_RECT ||
		this->current->type == TOOL_SELECT_REGION ||
		this->current->type == TOOL_SELECT_OBJECT ||
		this->current->type == TOOL_PLAY_OBJECT)
	{
		this->listener->toolColorChanged(false);
		this->listener->toolSizeChanged();
		this->listener->toolFillChanged();
		this->fireToolChanged();
	}
}

auto ToolHandler::isSinglePageTool() -> bool
{
	ToolType toolType = this->getToolType();
	DrawingType drawingType = this->getDrawingType();

	return toolType == (TOOL_PEN && (drawingType == DRAWING_TYPE_ARROW || drawingType == DRAWING_TYPE_CIRCLE || drawingType == DRAWING_TYPE_COORDINATE_SYSTEM
		|| drawingType == DRAWING_TYPE_LINE || drawingType == DRAWING_TYPE_RECTANGLE))
		|| toolType == TOOL_SELECT_REGION || toolType == TOOL_SELECT_RECT || toolType == TOOL_SELECT_OBJECT || toolType == TOOL_DRAW_RECT || toolType == TOOL_DRAW_CIRCLE
		|| toolType == TOOL_DRAW_COORDINATE_SYSTEM || toolType == TOOL_DRAW_ARROW|| toolType==TOOL_FLOATING_TOOLBOX;
}
