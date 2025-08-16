// TextPlaceholderTool.cpp
#include <algorithm>

#include "TextPlaceholderTool.h"


TextPlaceholderTool::TextPlaceholderTool(const std::string& name, TextPlaceholderConfig* config)
    : name(name), config(config) {}

std::string TextPlaceholderTool::getDisplayText() const {
    std::string value = config->getValue(name);
    if (value == "none" || value.empty()) {
        return name;
    }
    // Replace all newlines with spaces to ensure single-line display
    std::replace(value.begin(), value.end(), '\n', ' ');
    return value;
}

void TextPlaceholderTool::setValue(const std::string& value) {
    config->setValue(name, value);
    config->save();
}

std::string TextPlaceholderTool::getName() const {
    return name;
}
