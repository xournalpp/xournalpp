#include "Tool.h"

#include <optional>
#include <utility>

Tool::Tool(std::string name, ToolType type, Color color, unsigned int capabilities,
           std::optional<std::array<double, toolSizes>> thickness):
        name{std::move(name)}, type{type}, thickness{std::move(thickness)}, capabilities{capabilities} {
    setColor(color);
}

Tool::Tool(const Tool& t): name{t.name}, type{t.type}, thickness{t.thickness}, capabilities{t.capabilities} {
    setColor(t.getColor());
}

Tool::~Tool() {}

auto Tool::getName() const -> std::string { return this->name; }

auto Tool::getToolType() const -> ToolType { return this->type; }

void Tool::setCapability(unsigned int capability, bool enabled) {
    if (enabled) {
        this->capabilities |= capability;
    } else {
        this->capabilities &= ~capability;
    }
}

auto Tool::hasCapability(ToolCapabilities cap) const -> bool { return (this->capabilities & cap) != 0; }

auto Tool::getThickness(ToolSize size) const -> double { return this->thickness.value()[size - TOOL_SIZE_VERY_FINE]; }

auto Tool::isDrawingTool() const -> bool {
    return this->type == TOOL_PEN || this->type == TOOL_HIGHLIGHTER || this->type == TOOL_ERASER;
}
