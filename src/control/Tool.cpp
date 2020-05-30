#include "Tool.h"

#include <utility>

Tool::Tool(string name, ToolType type, Color color, int capabilities, double* thickness) {
    this->name = std::move(name);
    this->type = type;
    this->thickness = thickness;

    this->capabilities = capabilities;

    setColor(color);
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
