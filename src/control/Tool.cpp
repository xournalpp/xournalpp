#include "Tool.h"

// TODO: NEXT-RELEASE add fill tool
// TODO: NEXT-RELEASE allow grouping
// TODO: NEXT-RELEASE allow drag & drop of images and text
// TODO LOW PRIO select object: best match instead of first match
// TODO LOW PRIO: rouler every 90° latch

Tool::Tool(String name, ToolType type, int color, bool enableColor, bool enableSize, bool enableRuler, bool enableShapreRecognizer, double * thickness) {
	XOJ_INIT_TYPE(Tool);

	this->name = name;
	this->type = type;
	this->thickness = thickness;

	this->enableColor = enableColor;
	this->enableSize = enableSize;
	this->enableShapeRecognizer = enableShapreRecognizer;
	this->enableRuler = enableRuler;

	this->ruler = false;
	this->shapeRecognizer = false;

	this->color = color;
	this->size = TOOL_SIZE_MEDIUM;
}

Tool::~Tool() {
	XOJ_CHECK_TYPE(Tool);

	delete[] this->thickness;

	XOJ_RELEASE_TYPE(Tool);
}

String Tool::getName() {
	XOJ_CHECK_TYPE(Tool);

	return this->name;
}

int Tool::getColor() {
	XOJ_CHECK_TYPE(Tool);

	return this->color;
}

void Tool::setColor(int color) {
	XOJ_CHECK_TYPE(Tool);

	this->color = color;
}

ToolSize Tool::getSize() {
	XOJ_CHECK_TYPE(Tool);

	return this->size;
}

void Tool::setSize(ToolSize size) {
	XOJ_CHECK_TYPE(Tool);

	this->size = size;
}

bool Tool::isEnableColor() {
	XOJ_CHECK_TYPE(Tool);

	return this->enableColor;
}

bool Tool::isEnableSize() {
	XOJ_CHECK_TYPE(Tool);

	return this->enableSize;
}

bool Tool::isEnableRuler() {
	XOJ_CHECK_TYPE(Tool);

	return this->enableRuler;
}

bool Tool::isEnableShapeRecognizer() {
	XOJ_CHECK_TYPE(Tool);

	return this->enableShapeRecognizer;
}

bool Tool::isShapeRecognizer() {
	XOJ_CHECK_TYPE(Tool);

	return this->shapeRecognizer;
}

bool Tool::isRuler() {
	XOJ_CHECK_TYPE(Tool);

	return this->ruler;
}

void Tool::setShapeRecognizer(bool enabled) {
	XOJ_CHECK_TYPE(Tool);

	this->shapeRecognizer = enabled;
}

void Tool::setRuler(bool enabled) {
	XOJ_CHECK_TYPE(Tool);

	this->ruler = enabled;
}

String toolTypeToString(ToolType type) {
	switch (type) {
	case TOOL_NONE:
		return "none";
	case TOOL_PEN:
		return "pen";
	case TOOL_ERASER:
		return "eraser";
	case TOOL_HILIGHTER:
		return "hilighter";
	case TOOL_TEXT:
		return "text";
	case TOOL_IMAGE:
		return "image";
	case TOOL_SELECT_RECT:
		return "selectRect";
	case TOOL_SELECT_REGION:
		return "selectRegion";
	case TOOL_SELECT_OBJECT:
		return "selectObject";
	case TOOL_VERTICAL_SPACE:
		return "verticalSpace";
	case TOOL_HAND:
		return "hand";
	}
	return "";
}

ToolType toolTypeFromString(String type) {
	if (type == "none") {
		return TOOL_NONE;
	} else if (type == "pen") {
		return TOOL_PEN;
	} else if (type == "eraser") {
		return TOOL_ERASER;
	} else if (type == "hilighter") {
		return TOOL_HILIGHTER;
	} else if (type == "image") {
		return TOOL_IMAGE;
	} else if (type == "selectRect") {
		return TOOL_SELECT_RECT;
	} else if (type == "selectRegion") {
		return TOOL_SELECT_REGION;
	} else if (type == "selectObject") {
		return TOOL_SELECT_OBJECT;
	} else if (type == "verticalSpace") {
		return TOOL_VERTICAL_SPACE;
	} else if (type == "hand") {
		return TOOL_HAND;
	}

	return TOOL_NONE;
}

String toolSizeToString(ToolSize size) {
	switch (size) {
	case TOOL_SIZE_NONE:
		return "none";
	case TOOL_SIZE_VERY_FINE:
		return "veryThin";
	case TOOL_SIZE_FINE:
		return "thin";
	case TOOL_SIZE_MEDIUM:
		return "medium";
	case TOOL_SIZE_THICK:
		return "thick";
	case TOOL_SIZE_VERY_THICK:
		return "veryThick";
	}

	return "";
}

ToolSize toolSizeFromString(String size) {
	if (size == "veryThin") {
		return TOOL_SIZE_VERY_FINE;
	} else if (size == "thin") {
		return TOOL_SIZE_FINE;
	} else if (size == "medium") {
		return TOOL_SIZE_MEDIUM;
	} else if (size == "thick") {
		return TOOL_SIZE_THICK;
	} else if (size == "veryThick") {
		return TOOL_SIZE_VERY_THICK;
	} else if (size == "none") {
		return TOOL_SIZE_NONE;
	}

	return TOOL_SIZE_NONE;
}

String eraserTypeToString(EraserType type) {
	switch (type) {
	case ERASER_TYPE_NONE:
		return "none";
	case ERASER_TYPE_DEFAULT:
		return "default";
	case ERASER_TYPE_WHITEOUT:
		return "whiteout";
	case ERASER_TYPE_DELETE_STROKE:
		return "deleteStroke";
	}
	return "";
}

EraserType eraserTypeFromString(String type) {
	if (type == "none") {
		return ERASER_TYPE_NONE;
	} else if (type == "default") {
		return ERASER_TYPE_DEFAULT;
	} else if (type == "whiteout") {
		return ERASER_TYPE_WHITEOUT;
	} else if (type == "deleteStroke") {
		return ERASER_TYPE_DELETE_STROKE;
	}
	return ERASER_TYPE_NONE;
}

String pageInsertTypeToString(PageInsertType type) {
	if (type == PAGE_INSERT_TYPE_PLAIN) {
		return "plain";
	} else if (type == PAGE_INSERT_TYPE_LINED) {
		return "lined";
	} else if (type == PAGE_INSERT_TYPE_RULED) {
		return "ruled";
	} else if (type == PAGE_INSERT_TYPE_GRAPH) {
		return "graph";
	} else if (type == PAGE_INSERT_TYPE_COPY) {
		return "copyPage";
	} else if (type == PAGE_INSERT_TYPE_PDF_BACKGROUND) {
		return "pdfBackground";
	}
	return "";
}

PageInsertType pageInsertTypeFromString(String type) {
	if (type == "plain") {
		return PAGE_INSERT_TYPE_PLAIN;
	} else if (type == "lined") {
		return PAGE_INSERT_TYPE_LINED;
	} else if (type == "ruled") {
		return PAGE_INSERT_TYPE_RULED;
	} else if (type == "graph") {
		return PAGE_INSERT_TYPE_GRAPH;
	} else if (type == "copyPage") {
		return PAGE_INSERT_TYPE_COPY;
	} else if (type == "pdfBackground") {
		return PAGE_INSERT_TYPE_PDF_BACKGROUND;
	}

	return PAGE_INSERT_TYPE_COPY;
}

