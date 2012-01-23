#include "ToolHandler.h"
#include <stdio.h>
#include <Util.h>
#include "Actions.h"

#include <gtk/gtk.h>

ToolHandler::ToolHandler(ToolListener * listener, ActionHandler * actionHandler, Settings * settings) {
	XOJ_INIT_TYPE(ToolHandler);

	this->colorFound = false;
	this->listener = NULL;
	this->actionHandler = NULL;
	this->settings = settings;
	this->lastSelectedTool = NULL;
	initTools();
	this->listener = listener;
	this->actionHandler = actionHandler;

	this->eraserType = ERASER_TYPE_DEFAULT;
}

void ToolHandler::initTools() {
	XOJ_CHECK_TYPE(ToolHandler);

	Tool * t = NULL;

	for (int i = 0; i < TOOL_COUNT; i++) {
		tools[i] = NULL;
	}

	double * thickness = new double[5];
	// pen thicknesses = 0.15, 0.3, 0.5, 0.8, 2 mm
	thickness[TOOL_SIZE_VERY_FINE] = 0.42;
	thickness[TOOL_SIZE_FINE] = 0.85;
	thickness[TOOL_SIZE_MEDIUM] = 1.41;
	thickness[TOOL_SIZE_THICK] = 2.26;
	thickness[TOOL_SIZE_VERY_THICK] = 5.67;
	t = new Tool("pen", TOOL_PEN, 0x3333CC, true, true, true, true, thickness);
	tools[TOOL_PEN - TOOL_PEN] = t;

	thickness = new double[5];
	thickness[TOOL_SIZE_VERY_FINE] = 2.83;
	thickness[TOOL_SIZE_FINE] = 2.83;
	thickness[TOOL_SIZE_MEDIUM] = 8.50;
	thickness[TOOL_SIZE_THICK] = 19.84;
	thickness[TOOL_SIZE_VERY_THICK] = 19.84;
	t = new Tool("eraser", TOOL_ERASER, 0x000000, false, true, false, false, thickness);
	tools[TOOL_ERASER - TOOL_PEN] = t;

	// highlighter thicknesses = 1, 3, 7 mm
	thickness = new double[5];
	thickness[TOOL_SIZE_VERY_FINE] = 2.83;
	thickness[TOOL_SIZE_FINE] = 2.83;
	thickness[TOOL_SIZE_MEDIUM] = 8.50;
	thickness[TOOL_SIZE_THICK] = 19.84;
	thickness[TOOL_SIZE_VERY_THICK] = 19.84;
	t = new Tool("hilighter", TOOL_HILIGHTER, 0xFFFF00, true, true, true, true, thickness);
	tools[TOOL_HILIGHTER - TOOL_PEN] = t;

	t = new Tool("text", TOOL_TEXT, 0x000000, true, false, false, false, NULL);
	tools[TOOL_TEXT - TOOL_PEN] = t;

	t = new Tool("image", TOOL_IMAGE, 0x000000, false, false, false, false, NULL);
	tools[TOOL_IMAGE - TOOL_PEN] = t;

	t = new Tool("selectRect", TOOL_SELECT_RECT, 0x000000, false, false, false, false, NULL);
	tools[TOOL_SELECT_RECT - TOOL_PEN] = t;

	t = new Tool("selectRegion", TOOL_SELECT_REGION, 0x000000, false, false, false, false, NULL);
	tools[TOOL_SELECT_REGION - TOOL_PEN] = t;

	t = new Tool("selectObject", TOOL_SELECT_OBJECT, 0x000000, false, false, false, false, NULL);
	tools[TOOL_SELECT_OBJECT - TOOL_PEN] = t;

	t = new Tool("verticalSpace", TOOL_VERTICAL_SPACE, 0x000000, false, false, false, false, NULL);
	tools[TOOL_VERTICAL_SPACE - TOOL_PEN] = t;

	t = new Tool("hand", TOOL_HAND, 0x000000, false, false, false, false, NULL);
	tools[TOOL_HAND - TOOL_PEN] = t;

	selectTool(TOOL_PEN);
}

ToolHandler::~ToolHandler() {
	XOJ_CHECK_TYPE(ToolHandler);

	for (int i = 0; i < TOOL_COUNT; i++) {
		delete tools[i];
		tools[i] = NULL;
	}

	// Do not delete settings!
	this->settings = NULL;

	XOJ_RELEASE_TYPE(ToolHandler);
}

void ToolHandler::setEraserType(EraserType eraserType) {
	XOJ_CHECK_TYPE(ToolHandler);

	this->eraserType = eraserType;
	eraserTypeChanged();
}

void ToolHandler::eraserTypeChanged() {
	XOJ_CHECK_TYPE(ToolHandler);

	if (this->actionHandler == NULL) {
		return;
	}

	switch (this->eraserType) {
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

EraserType ToolHandler::getEraserType() {
	XOJ_CHECK_TYPE(ToolHandler);

	return this->eraserType;
}

void ToolHandler::selectTool(ToolType type, bool fireToolChanged) {
	XOJ_CHECK_TYPE(ToolHandler);

	if (type < 1 || type > TOOL_COUNT) {
		g_warning("unknown tool selected: %i\n", type);
		return;
	}
	this->current = tools[type - TOOL_PEN];

	if (fireToolChanged) {
		this->fireToolChanged();
	}
}

void ToolHandler::fireToolChanged() {
	XOJ_CHECK_TYPE(ToolHandler);

	if (listener) {
		listener->toolChanged();
	}
}

ToolType ToolHandler::getToolType() {
	XOJ_CHECK_TYPE(ToolHandler);

	return this->current->type;
}

bool ToolHandler::isEnableColor() {
	XOJ_CHECK_TYPE(ToolHandler);

	return current->enableColor;
}

bool ToolHandler::isEnableSize() {
	XOJ_CHECK_TYPE(ToolHandler);

	return current->enableSize;
}

bool ToolHandler::isEnableRuler() {
	XOJ_CHECK_TYPE(ToolHandler);

	return current->enableRuler;
}

bool ToolHandler::isEnableShapreRecognizer() {
	XOJ_CHECK_TYPE(ToolHandler);

	return current->enableShapeRecognizer;
}

void ToolHandler::setRuler(bool ruler) {
	XOJ_CHECK_TYPE(ToolHandler);

	this->current->ruler = ruler;
}

bool ToolHandler::isRuler() {
	XOJ_CHECK_TYPE(ToolHandler);

	return this->current->ruler;
}

void ToolHandler::setShapeRecognizer(bool reco) {
	XOJ_CHECK_TYPE(ToolHandler);

	this->current->shapeRecognizer = reco;
}

bool ToolHandler::isShapeRecognizer() {
	XOJ_CHECK_TYPE(ToolHandler);

	return this->current->shapeRecognizer;
}

ToolSize ToolHandler::getSize() {
	XOJ_CHECK_TYPE(ToolHandler);

	return this->current->size;
}

ToolSize ToolHandler::getPenSize() {
	XOJ_CHECK_TYPE(ToolHandler);

	return tools[TOOL_PEN - TOOL_PEN]->size;
}

ToolSize ToolHandler::getEraserSize() {
	XOJ_CHECK_TYPE(ToolHandler);

	return tools[TOOL_ERASER - TOOL_PEN]->size;
}

ToolSize ToolHandler::getHilighterSize() {
	XOJ_CHECK_TYPE(ToolHandler);

	return tools[TOOL_HILIGHTER - TOOL_PEN]->size;
}

void ToolHandler::setPenSize(ToolSize size) {
	XOJ_CHECK_TYPE(ToolHandler);

	this->tools[TOOL_PEN - TOOL_PEN]->size = size;

	if (this->current->type == TOOL_PEN) {
		this->listener->toolSizeChanged();
	}
}

void ToolHandler::setEraserSize(ToolSize size) {
	XOJ_CHECK_TYPE(ToolHandler);

	this->tools[TOOL_ERASER - TOOL_PEN]->size = size;

	if (this->current->type == TOOL_ERASER) {
		this->listener->toolSizeChanged();
	}
}

void ToolHandler::setHilighterSize(ToolSize size) {
	XOJ_CHECK_TYPE(ToolHandler);

	this->tools[TOOL_HILIGHTER - TOOL_PEN]->size = size;

	if (this->current->type == TOOL_HILIGHTER) {
		this->listener->toolSizeChanged();
	}
}

double ToolHandler::getThickness() {
	XOJ_CHECK_TYPE(ToolHandler);

	if (this->current->thickness) {
		return this->current->thickness[this->current->size];
	} else {
		g_warning("Request size of \"%s\"", this->current->getName().c_str());
		return 0;
	}
}

void ToolHandler::setSize(ToolSize size) {
	XOJ_CHECK_TYPE(ToolHandler);

	if (size < TOOL_SIZE_VERY_FINE || size > TOOL_SIZE_VERY_THICK) {
		g_warning("ToolHandler::setSize: Invalid size! %i", size);
		return;
	}

	this->current->size = size;
	this->listener->toolSizeChanged();
}

void ToolHandler::setColor(int color) {
	XOJ_CHECK_TYPE(ToolHandler);

	this->colorFound = false;

	this->current->color = color;
	this->listener->toolColorChanged();

	if (!colorFound) {
		this->listener->setCustomColorSelected();
	}
}

int ToolHandler::getColor() {
	XOJ_CHECK_TYPE(ToolHandler);

	return current->color;
}

GdkColor ToolHandler::getGdkColor() {
	XOJ_CHECK_TYPE(ToolHandler);

	return Util::intToGdkColor(this->current->color);
}

void ToolHandler::setColorFound() {
	XOJ_CHECK_TYPE(ToolHandler);

	this->colorFound = true;
}

ArrayIterator<Tool *> ToolHandler::iterator() {
	XOJ_CHECK_TYPE(ToolHandler);

	return ArrayIterator<Tool *> (tools, TOOL_COUNT);
}

void ToolHandler::saveSettings() {
	XOJ_CHECK_TYPE(ToolHandler);

	SElement & s = settings->getCustomElement("tools");
	s.clear();

	s.setString("current", this->current->getName());

	ArrayIterator<Tool *> it = iterator();

	for (; it.hasNext();) {
		Tool * t = it.next();
		SElement & st = s.child(t->getName());
		if (t->isEnableColor()) {
			st.setIntHex("color", t->getColor());
		}
		if (t->isEnableRuler()) {
			st.setBool("ruler", t->isRuler());
		}
		if (t->isEnableShapeRecognizer()) {
			st.setBool("shapeRecognizer", t->isShapeRecognizer());
		}

		if (t->isEnableSize()) {
			String value;
			switch (t->getSize()) {
			case TOOL_SIZE_VERY_FINE:
				value = "VERY_THIN";
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

		if (t->type == TOOL_ERASER) {
			if (this->eraserType == ERASER_TYPE_DELETE_STROKE) {
				st.setString("type", "deleteStroke");
			} else if (this->eraserType == ERASER_TYPE_WHITEOUT) {
				st.setString("type", "whiteout");
			} else {//ERASER_TYPE_DEFAULT
				st.setString("type", "default");
			}
		}
	}

	settings->customSettingsChanged();
}

void ToolHandler::loadSettings() {
	XOJ_CHECK_TYPE(ToolHandler);

	SElement & s = settings->getCustomElement("tools");

	String selectedTool;
	s.getString("current", selectedTool);

	ArrayIterator<Tool *> it = iterator();

	for (; it.hasNext();) {
		Tool * t = it.next();
		SElement & st = s.child(t->getName());

		if (selectedTool == t->getName()) {
			this->current = t;
		}

		int color = 0;
		if (t->isEnableColor() && st.getInt("color", color)) {
			t->setColor(color);
		}

		bool enabled = false;
		if (t->isEnableRuler() && st.getBool("ruler", enabled)) {
			t->setRuler(enabled);
		}
		if (t->isEnableShapeRecognizer() && st.getBool("shapeRecognizer", enabled)) {
			t->setShapeRecognizer(enabled);
		}

		String value;

		if (t->isEnableSize() && st.getString("size", value)) {
			if (value.equals("VERY_THIN")) {
				t->setSize(TOOL_SIZE_VERY_FINE);
			} else if (value.equals("THIN")) {
				t->setSize(TOOL_SIZE_FINE);
			} else if (value.equals("MEDIUM")) {
				t->setSize(TOOL_SIZE_MEDIUM);
			} else if (value.equals("BIG")) {
				t->setSize(TOOL_SIZE_THICK);
			} else if (value.equals("VERY_BIG")) {
				t->setSize(TOOL_SIZE_VERY_THICK);
			} else {
				g_warning("Settings::Unknown tool size: %s\n", value.c_str());
			}
		}

		if (t->type == TOOL_ERASER) {
			String type;

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

void ToolHandler::copyCurrentConfig() {
	XOJ_CHECK_TYPE(ToolHandler);

	this->lastSelectedTool = this->current;
}

void ToolHandler::restoreLastConfig() {
	XOJ_CHECK_TYPE(ToolHandler);

	if (this->lastSelectedTool == NULL) {
		return;
	}
	this->current = this->lastSelectedTool;
	this->lastSelectedTool = NULL;

	this->listener->toolColorChanged();
	this->listener->toolSizeChanged();
	this->fireToolChanged();
}

const double * ToolHandler::getToolThickness(ToolType type) {
	XOJ_CHECK_TYPE(ToolHandler);

	return this->tools[type - TOOL_PEN]->thickness;
}

void ToolHandler::setSelectionEditTools(bool setColor, bool setSize) {
	XOJ_CHECK_TYPE(ToolHandler);

	for (int i = TOOL_SELECT_RECT - TOOL_PEN; i <= TOOL_SELECT_OBJECT - TOOL_PEN; i++) {
		Tool * t = tools[i];
		t->enableColor = setColor;
		t->enableSize = setSize;
		t->size = TOOL_SIZE_NONE;
		t->color = -1;
	}

	if (this->current->type == TOOL_SELECT_RECT || this->current->type == TOOL_SELECT_REGION || this->current->type == TOOL_SELECT_OBJECT) {
		this->listener->toolColorChanged();
		this->listener->toolSizeChanged();
		this->fireToolChanged();
	}
}
