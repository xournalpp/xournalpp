// TextPlaceholderTool.h
#pragma once
#include <string>
#include "config/TextPlaceholderConfig.h"

class TextPlaceholderTool {
public:
    TextPlaceholderTool(const std::string& name, TextPlaceholderConfig* config);
    std::string getDisplayText() const;
    void setValue(const std::string& value);
    std::string getName() const;
private:
    std::string name;
    TextPlaceholderConfig* config;
};
