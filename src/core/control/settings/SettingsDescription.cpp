#include "SettingsDescription.h"

#include <array>
#include <map>
#include <memory>

#include "model/Font.h"

#include "ButtonConfig.h"
#include "LatexSettings.h"
#include "Settings.h"
#include "SettingsEnums.h"

// Initialize non literal DEFAULT values here
const Setting<SettingsElement::FONT>::value_type Setting<SettingsElement::FONT>::DEFAULT = XojFont("Sans", 12);
const Setting<SettingsElement::DEFAULT_SAVE_NAME>::value_type Setting<SettingsElement::DEFAULT_SAVE_NAME>::DEFAULT =
        _("%F-Note-%H-%M");
const Setting<SettingsElement::DEFAULT_PDF_EXPORT_NAME>::value_type
        Setting<SettingsElement::DEFAULT_PDF_EXPORT_NAME>::DEFAULT = _("%{name}_annotated");
const Setting<SettingsElement::LATEX_SETTINGS>::value_type Setting<SettingsElement::LATEX_SETTINGS>::DEFAULT{};
const Setting<SettingsElement::NESTED_BUTTON_CONFIG>::value_type
        Setting<SettingsElement::NESTED_BUTTON_CONFIG>::DEFAULT{
                std::make_shared<ButtonConfig>(TOOL_ERASER, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT,
                                               ERASER_TYPE_NONE),  // Eraser
                std::make_shared<ButtonConfig>(TOOL_HAND, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT,
                                               ERASER_TYPE_NONE),  // Middle button
                std::make_shared<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT,
                                               ERASER_TYPE_NONE),  // Right button
                std::make_shared<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT,
                                               ERASER_TYPE_NONE),  // Touch
                std::make_shared<ButtonConfig>(TOOL_PEN, Colors::black, TOOL_SIZE_FINE, DRAWING_TYPE_DEFAULT,
                                               ERASER_TYPE_NONE),  // Default config
                std::make_shared<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT,
                                               ERASER_TYPE_NONE),  // Pen button 1
                std::make_shared<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT,
                                               ERASER_TYPE_NONE)  // Pen button 2
        };
const Setting<SettingsElement::NESTED_DEVICE_CLASSES>::value_type
        Setting<SettingsElement::NESTED_DEVICE_CLASSES>::DEFAULT{};
const Setting<SettingsElement::NESTED_TOOLS>::value_type Setting<SettingsElement::NESTED_TOOLS>::DEFAULT{};
const Setting<SettingsElement::NESTED_TOUCH>::value_type Setting<SettingsElement::NESTED_TOUCH>::DEFAULT{};
const Setting<SettingsElement::NESTED_LAST_USED_PAGE_BACKGROUND_COLOR>::value_type
        Setting<SettingsElement::NESTED_LAST_USED_PAGE_BACKGROUND_COLOR>::DEFAULT{};
