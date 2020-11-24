#include "Tool.h"

#include <utility>

Tool::Tool(string name, ToolType type, Color color, int capabilities, double* thickness) {
    this->name = std::move(name);
    this->type = type;
    this->thickness = thickness;

    this->capabilities = capabilities;

    setColor(color);
}

Tool::Tool(Tool* t): name{t->name}, type{t->type}, capabilities{t->capabilities} {
    if (t->thickness) {
        this->thickness = new double[toolSizes];
        for (int i{0}; i < toolSizes; i++) {
            this->thickness[i] = t->thickness[i];
        }
    } else {
        this->thickness = nullptr;
    }
    setColor(t->getColor());
}

Tool::~Tool() {
    delete[] this->thickness;
    this->thickness = nullptr;
}

auto Tool::getName() -> string { return this->name; }

void Tool::setCapability(int capability, bool enabled) {
    if (enabled) {
        this->capabilities |= capability;
    } else {
        this->capabilities &= ~capability;
    }
}

auto Tool::hasCapability(ToolCapabilities cap) const -> bool { return (this->capabilities & cap) != 0; }

auto Tool::getThickness(ToolSize size) -> double { return this->thickness[size - TOOL_SIZE_VERY_FINE]; }

auto Tool::isDrawingTool() -> bool {
    return this->type == TOOL_PEN || this->type == TOOL_HIGHLIGHTER || this->type == TOOL_ERASER;
}
