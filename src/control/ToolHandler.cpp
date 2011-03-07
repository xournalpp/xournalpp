#include "ToolHandler.h"
#include <stdio.h>
#include "../util/Util.h"

#include <gtk/gtk.h>

ToolHandler::ToolHandler(ToolListener * listener, Settings * settings) {
	colorFound = false;
	this->listener = NULL;
	this->settings = settings;
	this->lastSelectedTool = NULL;
	initTools();
	this->listener = listener;

	this->eraserType = ERASER_TYPE_DEFAULT;
}

void ToolHandler::initTools() {
	Tool * t;

	for (int i = 0; i < TOOL_COUNT; i++) {
		tools[i] = NULL;
	}

	double * thikness = new double[5];
	// pen thicknesses = 0.15, 0.3, 0.5, 0.8, 2 mm
	thikness[TOOL_SIZE_VERY_FINE] = 0.42;
	thikness[TOOL_SIZE_FINE] = 0.85;
	thikness[TOOL_SIZE_MEDIUM] = 1.41;
	thikness[TOOL_SIZE_THICK] = 2.26;
	thikness[TOOL_SIZE_VERY_THICK] = 5.67;
	t = new Tool("pen", TOOL_PEN, 0x3333CC, true, true, true, true, thikness);
	tools[TOOL_PEN - TOOL_PEN] = t;

	thikness = new double[5];
	thikness[TOOL_SIZE_VERY_FINE] = 2.83;
	thikness[TOOL_SIZE_FINE] = 2.83;
	thikness[TOOL_SIZE_MEDIUM] = 8.50;
	thikness[TOOL_SIZE_THICK] = 19.84;
	thikness[TOOL_SIZE_VERY_THICK] = 19.84;
	t = new Tool("eraser", TOOL_ERASER, 0x000000, false, true, false, false, thikness);
	tools[TOOL_ERASER - TOOL_PEN] = t;

	// highlighter thicknesses = 1, 3, 7 mm
	thikness = new double[5];
	thikness[TOOL_SIZE_VERY_FINE] = 2.83;
	thikness[TOOL_SIZE_FINE] = 2.83;
	thikness[TOOL_SIZE_MEDIUM] = 8.50;
	thikness[TOOL_SIZE_THICK] = 19.84;
	thikness[TOOL_SIZE_VERY_THICK] = 19.84;
	t = new Tool("hilighter", TOOL_HILIGHTER, 0xFFFF00, true, true, true, true, thikness);
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
	for (int i = 0; i < TOOL_COUNT; i++) {
		delete tools[i];
	}
	// Do not delete settings!
	this->settings = NULL;
}

void ToolHandler::_setEraserType(EraserType eraserType) {
	this->eraserType = eraserType;
}

EraserType ToolHandler::getEraserType() {
	return this->eraserType;
}

void ToolHandler::selectTool(ToolType type) {
	if (type < 1 || type > TOOL_COUNT) {
		g_warning("unknown tool selected: %i\n", type);
		return;
	}
	this->current = tools[type - TOOL_PEN];

	if (listener) {
		listener->toolChanged();
	}
}

ToolType ToolHandler::getToolType() {
	return this->current->type;
}

bool ToolHandler::isEnableColor() {
	return current->enableColor;
}

bool ToolHandler::isEnableSize() {
	return current->enableSize;
}

bool ToolHandler::isEnableRuler() {
	return current->enableRuler;
}

bool ToolHandler::isEnableShapreRecognizer() {
	return current->enableShapeRecognizer;
}

void ToolHandler::setRuler(bool ruler) {
	this->current->ruler = ruler;
}

bool ToolHandler::isRuler() {
	return this->current->ruler;
}

void ToolHandler::setShapeRecognizer(bool reco) {
	this->current->shapeRecognizer = reco;
}

bool ToolHandler::isShapeRecognizer() {
	return this->current->shapeRecognizer;
}

ToolSize ToolHandler::getSize() {
	return this->current->size;
}

ToolSize ToolHandler::getPenSize() {
	return tools[TOOL_PEN - TOOL_PEN]->size;
}

ToolSize ToolHandler::getEraserSize() {
	return tools[TOOL_ERASER - TOOL_PEN]->size;
}

ToolSize ToolHandler::getHilighterSize() {
	return tools[TOOL_HILIGHTER - TOOL_PEN]->size;
}

void ToolHandler::setPenSize(ToolSize size) {
	tools[TOOL_PEN - TOOL_PEN]->size = size;

	if (this->current->type == TOOL_PEN) {
		listener->toolSizeChanged();
	}
}

void ToolHandler::setEraserSize(ToolSize size) {
	tools[TOOL_ERASER - TOOL_PEN]->size = size;

	if (this->current->type == TOOL_ERASER) {
		listener->toolSizeChanged();
	}
}

void ToolHandler::setHilighterSize(ToolSize size) {
	tools[TOOL_HILIGHTER - TOOL_PEN]->size = size;

	if (this->current->type == TOOL_HILIGHTER) {
		listener->toolSizeChanged();
	}
}

double ToolHandler::getThikness() {
	if (this->current->thikness) {
		return this->current->thikness[this->current->size];
	} else {
		g_warning("Request size of \"%s\"", this->current->getName().c_str());
		return 0;
	}
}

void ToolHandler::setSize(ToolSize size) {
	if (size < TOOL_SIZE_VERY_FINE || size > TOOL_SIZE_VERY_THICK) {
		g_warning("ToolHandler::setSize: Invalid size! %i", size);
		return;
	}

	this->current->size = size;
	listener->toolSizeChanged();
}

void ToolHandler::setColor(int color) {
	colorFound = false;

	current->color = color;
	listener->toolColorChanged();

	if (!colorFound) {
		listener->setCustomColorSelected();
	}
}

int ToolHandler::getColor() {
	return current->color;
}

GdkColor ToolHandler::getGdkColor() {
	return Util::intToGdkColor(current->color);
}

void ToolHandler::setColorFound() {
	colorFound = true;
}

ArrayIterator<Tool *> ToolHandler::iterator() {
	return ArrayIterator<Tool *> (tools, TOOL_COUNT);
}

void ToolHandler::saveSettings() {
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
					_setEraserType(ERASER_TYPE_DELETE_STROKE);
				} else if (type == "whiteout") {
					_setEraserType(ERASER_TYPE_WHITEOUT);
				} else {
					_setEraserType(ERASER_TYPE_DEFAULT);
				}
				listener->eraserTypeChanged();
			}
		}
	}

}

void ToolHandler::copyCurrentConfig() {
	this->lastSelectedTool = this->current;
}

void ToolHandler::restoreLastConfig() {
	if (this->lastSelectedTool == NULL) {
		return;
	}
	this->current = this->lastSelectedTool;
	this->lastSelectedTool = NULL;

	listener->toolColorChanged();
	listener->toolSizeChanged();
	listener->toolChanged();
}

const double * ToolHandler::getToolThikness(ToolType type) {
	return tools[type - TOOL_PEN]->thikness;
}

void ToolHandler::setSelectionEditTools(bool setColor, bool setSize) {
	for (int i = TOOL_SELECT_RECT - TOOL_PEN; i <= TOOL_SELECT_OBJECT - TOOL_PEN; i++) {
		Tool * t = tools[i];
		t->enableColor = setColor;
		t->enableSize = setSize;
		t->size = TOOL_SIZE_NONE;
		t->color = -1;
	}

	if (this->current->type == TOOL_SELECT_RECT || this->current->type == TOOL_SELECT_RECT || this->current->type == TOOL_SELECT_OBJECT) {
		listener->toolColorChanged();
		listener->toolSizeChanged();
		listener->toolChanged();
	}
}
