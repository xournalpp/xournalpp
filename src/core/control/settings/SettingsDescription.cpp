#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <libxml/tree.h>

#include "control/ToolEnums.h"
#include "model/Font.h"
#include "util/Color.h"
#include "util/i18n.h"  // for _

#include "ButtonConfig.h"
#include "LatexSettings.h"
#include "Settings.h"
#include "SettingsEnums.h"
#include "ViewModes.h"
#include "filesystem.h"

// clang-format off
#include "SettingsDescription.h"
// clang-format on

// Initialize non literal DEFAULT values here
Setting<SettingsElement::SETTING_FONT>::value_type Setting<SettingsElement::SETTING_FONT>::DEFAULT =
        XojFont("Sans", 12);
Setting<SettingsElement::SETTING_DEFAULT_SAVE_NAME>::value_type
        Setting<SettingsElement::SETTING_DEFAULT_SAVE_NAME>::DEFAULT = _("%F-Note-%H-%M");
Setting<SettingsElement::SETTING_DEFAULT_PDF_EXPORT_NAME>::value_type
        Setting<SettingsElement::SETTING_DEFAULT_PDF_EXPORT_NAME>::DEFAULT = _("%{name}_annotated");
Setting<SettingsElement::SETTING_LATEX_SETTINGS>::value_type
        Setting<SettingsElement::SETTING_LATEX_SETTINGS>::DEFAULT{};
Setting<SettingsElement::SETTING_NESTED_BUTTON_CONFIG>::value_type
        Setting<SettingsElement::SETTING_NESTED_BUTTON_CONFIG>::DEFAULT{
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
Setting<SettingsElement::SETTING_NESTED_DEVICE_CLASSES>::value_type
        Setting<SettingsElement::SETTING_NESTED_DEVICE_CLASSES>::DEFAULT{};
Setting<SettingsElement::SETTING_NESTED_TOOLS>::value_type Setting<SettingsElement::SETTING_NESTED_TOOLS>::DEFAULT{};
Setting<SettingsElement::SETTING_NESTED_TOUCH>::value_type Setting<SettingsElement::SETTING_NESTED_TOUCH>::DEFAULT{};
Setting<SettingsElement::SETTING_NESTED_LAST_USED_PAGE_BACKGROUND_COLOR>::value_type
        Setting<SettingsElement::SETTING_NESTED_LAST_USED_PAGE_BACKGROUND_COLOR>::DEFAULT{};


// Implementation of import functions
bool importStringProperty(xmlNodePtr node, std::string& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = reinterpret_cast<const char*>(value);
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importPathProperty(xmlNodePtr node, fs::path& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = reinterpret_cast<const char*>(value);
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importBoolProperty(xmlNodePtr node, bool& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importDoubleProperty(xmlNodePtr node, double& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importIntProperty(xmlNodePtr node, int& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = static_cast<int>(g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10));
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importUintProperty(xmlNodePtr node, uint& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = static_cast<uint>(g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10));
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importColorProperty(xmlNodePtr node, Color& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = ColorU8(static_cast<uint>(g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10)));
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importULintProperty(xmlNodePtr node, size_t& var) {
    xmlChar* value = xmlGetProp(node, reinterpret_cast<const xmlChar*>("value"));
    if (value) {
        var = g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10);
        xmlFree(value);
        return true;
    }
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    g_warning("SettingDescription::No value for property '%s'!\n", reinterpret_cast<const char*>(name));
    xmlFree(name);
    return false;
}
bool importFont(xmlNodePtr node, XojFont& var) {
    xmlChar* font = xmlGetProp(node, reinterpret_cast<const xmlChar*>("font"));
    xmlChar* size = xmlGetProp(node, reinterpret_cast<const xmlChar*>("size"));
    if (font == nullptr || size == nullptr) {
        g_warning("SettingDescription::Missing value for property 'font'!\n");
        return false;
    }
    var.setName(reinterpret_cast<const char*>(font));
    var.setSize(g_ascii_strtod(reinterpret_cast<const char*>(size), nullptr));
    xmlFree(font);
    xmlFree(size);
    return true;
}
bool importSidebarNumbering(xmlNodePtr node, SidebarNumberingStyle& var) {
    int i = 0;
    if (importIntProperty(node, i)) {
        if (i > static_cast<int>(SidebarNumberingStyle::MIN) && i < static_cast<int>(SidebarNumberingStyle::MAX)) {
            var = static_cast<SidebarNumberingStyle>(i);
            return true;
        }
        g_warning("SettingsDescription::Invalid sidebarNumberingStyle value. Reset to default.");
    }
    return false;
}
bool importStylusCursorType(xmlNodePtr node, StylusCursorType& var) {
    std::string s = "";
    if (importStringProperty(node, s)) {
        var = stylusCursorTypeFromString(s);
        return true;
    }
    return false;
}
bool importEraserVisibility(xmlNodePtr node, EraserVisibility& var) {
    std::string s = "";
    if (importStringProperty(node, s)) {
        var = eraserVisibilityFromString(s);
        return true;
    }
    return false;
}
bool importIconTheme(xmlNodePtr node, IconTheme& var) {
    std::string s = "";
    if (importStringProperty(node, s)) {
        var = iconThemeFromString(s);
        return true;
    }
    return false;
}
bool importViewMode(xmlNodePtr node, ViewMode& var) {
    std::string s = "";
    if (importStringProperty(node, s)) {
        var = settingsStringToViewMode(s);
        return true;
    }
    return false;
}
bool importScrollbarHideType(xmlNodePtr node, ScrollbarHideType& var) {
    std::string s = "";
    if (importStringProperty(node, s)) {
        var = scrollbarHideTypeFromString(s);
        return true;
    }
    return false;
}
bool importPADeviceIndex(xmlNodePtr node, PaDeviceIndex& var) {
    int i = 0;
    if (importIntProperty(node, i)) {
        var = static_cast<PaDeviceIndex>(i);
        return true;
    }
    return false;
}
bool importEmptyLastPageAppendType(xmlNodePtr node, EmptyLastPageAppendType& var) {
    std::string s = "";
    if (importStringProperty(node, s)) {
        var = emptyLastPageAppendFromString(s);
        return true;
    }
    return false;
}
bool importLatexSettings(xmlNodePtr node, LatexSettings& var) {
    xmlChar* name = xmlGetProp(node, reinterpret_cast<const xmlChar*>("name"));
    std::string valName = reinterpret_cast<const char*>(name);
    xmlFree(name);

    if (valName == "latexSettings.autoCheckDependencies") {
        return importBoolProperty(node, var.autoCheckDependencies);
    }
    if (valName == "latexSettings.defaultText") {
        return importStringProperty(node, var.defaultText);
    }
    if (valName == "latexSettings.globalTemplatePath") {
        std::string globalTemplatePath;
        if (importStringProperty(node, globalTemplatePath)) {
            var.globalTemplatePath = globalTemplatePath;
            return true;
        }
        return false;
    }
    if (valName == "latexSettings.genCmd") {
        return importStringProperty(node, var.genCmd);
    }
    if (valName == "latexSettings.sourceViewThemeId") {
        return importStringProperty(node, var.sourceViewThemeId);
    }
    if (valName == "latexSettings.editorFont") {
        std::string editorFont;
        if (importStringProperty(node, editorFont)) {
            var.editorFont = editorFont;
            return true;
        }
        return false;
    }
    if (valName == "latexSettings.useCustomEditorFont") {
        return importBoolProperty(node, var.useCustomEditorFont);
    }
    if (valName == "latexSettings.editorWordWrap") {
        return importBoolProperty(node, var.editorWordWrap);
    }
    if (valName == "latexSettings.sourceViewAutoIndent") {
        return importBoolProperty(node, var.sourceViewAutoIndent);
    }
    if (valName == "latexSettings.sourceViewSyntaxHighlight") {
        return importBoolProperty(node, var.sourceViewSyntaxHighlight);
    }
    if (valName == "latexSettings.sourceViewShowLineNumbers") {
        return importBoolProperty(node, var.sourceViewShowLineNumbers);
    }

    g_warning("SettingsDescription::Unknown Latex property '%s'!", valName.c_str());
    return false;
}
bool importStrokeAveragingMethod(xmlNodePtr node, StrokeStabilizer::AveragingMethod& var) {
    int i = 0;
    if (importIntProperty(node, i)) {
        var = static_cast<StrokeStabilizer::AveragingMethod>(i);
        return true;
    }
    return false;
}
bool importStrokePreprocessor(xmlNodePtr node, StrokeStabilizer::Preprocessor& var) {
    int i = 0;
    if (importIntProperty(node, i)) {
        var = static_cast<StrokeStabilizer::Preprocessor>(i);
        return true;
    }
    return false;
}
bool importButtonConfig(xmlNodePtr node, std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& var) {
    std::map<std::string, std::pair<std::string, std::string>> buttonConfigMap{};
    for (xmlNodePtr cur = node->children; cur != nullptr; cur = cur->next) {  // Loop through the buttons
        if (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("data")) == 0) {
            std::string name;
            xmlChar* nameProp = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("name"));
            if (nameProp == nullptr) {
                g_warning("SettingsDescription::data tag missing name in buttonConfig!");
                continue;
            }
            name = reinterpret_cast<const char*>(nameProp);
            xmlFree(nameProp);

            for (xmlNodePtr attr = cur->children; attr != nullptr;
                 attr = attr->next) {  // Loop through the attributes and put them into the map
                if (xmlStrcmp(attr->name, reinterpret_cast<const xmlChar*>("attribute")) == 0) {
                    xmlChar* attrName = xmlGetProp(attr, reinterpret_cast<const xmlChar*>("name"));
                    xmlChar* value = xmlGetProp(attr, reinterpret_cast<const xmlChar*>("value"));
                    xmlChar* type = xmlGetProp(attr, reinterpret_cast<const xmlChar*>("type"));

                    if (attrName == nullptr || value == nullptr || type == nullptr) {
                        g_warning("SettingsDescription::attribute tag in buttonConfig for button '%s' is missing "
                                  "properties!",
                                  name.c_str());
                        xmlFree(attrName);
                        xmlFree(value);
                        xmlFree(type);
                        continue;
                    }

                    buttonConfigMap.emplace(
                            reinterpret_cast<const char*>(attrName),
                            std::make_pair(reinterpret_cast<const char*>(type), reinterpret_cast<const char*>(value)));
                    xmlFree(attrName);
                    xmlFree(value);
                    xmlFree(type);
                } else {
                    g_warning("SettingsDescription::Unexpected xml tag '%s' in buttonConfig for button '%s'!",
                              reinterpret_cast<const char*>(cur->name), name.c_str());
                }
            }

            // Now interpret the attributes saved in the map and build the button config
            auto btn = buttonFromString(name);
            if (btn) {
                const auto& cfg = var[static_cast<Button>(btn.value())];

                // Check if the tool type was read, if not skip this iteration
                if (auto pTool = buttonConfigMap.find("tool"); pTool != buttonConfigMap.end()) {
                    ToolType tool = TOOL_NONE;
                    if (pTool->second.first !=
                        "string") {  // Check if tool type has a type of string, if not leave type none
                        g_warning(
                                "SettingsDescription::ButtonConfig unexpected type for attribute tool for button '%s'",
                                reinterpret_cast<const char*>(cur->name));
                    } else {
                        tool = toolTypeFromString(pTool->second.second);
                    }
                    cfg->action = tool;

                    if (tool == TOOL_PEN) {  // If the tool is a pen, check for strokeType
                        auto strokeTypePair = buttonConfigMap.find("strokeType");
                        StrokeType strokeType = STROKE_TYPE_NONE;
                        if (strokeTypePair != buttonConfigMap.end()) {
                            if (strokeTypePair->second.first != "string") {
                                g_warning("SettingsDescription::ButtonConfig unexpected type for attribute strokeType "
                                          "for button '%s'",
                                          reinterpret_cast<const char*>(cur->name));
                            } else {
                                strokeType = strokeTypeFromString(strokeTypePair->second.second);
                            }
                        }
                        cfg->strokeType = strokeType;
                    }

                    if (tool == TOOL_PEN ||
                        tool == TOOL_HIGHLIGHTER) {  // For Pen and highlighter check for drawingType
                        if (auto drawingTypePair = buttonConfigMap.find("drawingType");
                            drawingTypePair != buttonConfigMap.end()) {
                            if (drawingTypePair->second.first !=
                                "string") {  // If type is not string ignore and read string anyways, but warn about it
                                g_warning("SettingsDescription::ButtonConfig unexpected type for attribute drawingType "
                                          "for button '%s'",
                                          reinterpret_cast<const char*>(cur->name));
                            }
                            cfg->drawingType = drawingTypeFromString(drawingTypePair->second.second);
                        }
                    }

                    if (tool == TOOL_PEN || tool == TOOL_HIGHLIGHTER || tool == TOOL_ERASER) {  // Check for size
                        auto sizePair = buttonConfigMap.find("size");
                        ToolSize size = TOOL_SIZE_NONE;
                        if (sizePair != buttonConfigMap.end()) {
                            if (sizePair->second.first != "string") {
                                g_warning("SettingsDescription::ButtonConfig unexpected type for attribute size for "
                                          "button '%s'",
                                          reinterpret_cast<const char*>(cur->name));
                            }
                            size = toolSizeFromString(sizePair->second.second);
                        }
                        cfg->size = size;
                    }

                    if (tool == TOOL_PEN || tool == TOOL_HIGHLIGHTER || tool == TOOL_TEXT) {  // Check for color
                        if (auto colorPair = buttonConfigMap.find("color"); colorPair != buttonConfigMap.end()) {
                            if (colorPair->second.first == "int") {
                                cfg->color = Color(static_cast<uint>(
                                        g_ascii_strtoull(colorPair->second.second.c_str(), nullptr, 10)));
                            } else if (colorPair->second.first == "hex") {
                                cfg->color = Color(static_cast<uint>(
                                        g_ascii_strtoull(colorPair->second.second.c_str(), nullptr, 16)));
                            } else {
                                g_warning("SettingsDescription::ButtonConfig unexpected type for attribute size for "
                                          "button '%s'",
                                          reinterpret_cast<const char*>(cur->name));
                            }
                        }
                    }

                    // Check for eraser mode
                    if (tool == TOOL_ERASER) {
                        auto eraserModePair = buttonConfigMap.find("eraserMode");
                        EraserType eraserMode = ERASER_TYPE_NONE;
                        if (eraserModePair != buttonConfigMap.end()) {
                            if (eraserModePair->second.first != "string") {
                                g_warning("SettingsDescription::ButtonConfig unexpected type for attribute eraserMode "
                                          "for button '%s'",
                                          reinterpret_cast<const char*>(cur->name));
                            }
                            eraserMode = eraserTypeFromString(eraserModePair->second.second);
                        }
                        cfg->eraserMode = eraserMode;
                    }

                    // Check for touch device now
                    if (static_cast<Button>(btn.value()) == BUTTON_TOUCH) {
                        auto devicePair = buttonConfigMap.find("device");
                        if (devicePair != buttonConfigMap.end()) {
                            if (devicePair->second.first != "string") {
                                g_warning("SettingsDescription::ButtonConfig unexpected type for attribute device for "
                                          "button '%s'",
                                          reinterpret_cast<const char*>(cur->name));
                            }
                            cfg->device = devicePair->second.second;
                        }

                        auto disableDrawingPair = buttonConfigMap.find("disableDrawing");
                        if (disableDrawingPair != buttonConfigMap.end()) {
                            if (disableDrawingPair->second.first != "boolean") {
                                g_warning("SettingsDescription::ButtonConfig unexpected type for attribute "
                                          "disableDrawing for button '%s'",
                                          reinterpret_cast<const char*>(cur->name));
                            } else {
                                cfg->disableDrawing = disableDrawingPair->second.second == "true";
                            }
                        }
                    }

                } else {
                    g_warning("SettingsDescription::ButtonConfig for '%s' does not contain tool attribute",
                              reinterpret_cast<const char*>(cur->name));
                }

            } else {
                g_warning("SettingsDescription::Unknown Button '%s'!", name.c_str());
            }

            // Finally clear map for next iteration
            buttonConfigMap.clear();
        } else {
            g_warning("SettingsDescription::Unexpected xml tag '%s' in buttonConfig!",
                      reinterpret_cast<const char*>(cur->name));
        }
    }
    return true;
}
bool importDeviceClasses(xmlNodePtr node,
                         std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& var) {
    for (xmlNodePtr cur = node->children; cur != nullptr; cur = cur->next) {  // Loop through the devices
        if (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("data")) == 0) {
            std::string name;
            std::optional<int> deviceClass{};
            std::optional<int> deviceSource{};

            xmlChar* nameProp = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("name"));
            if (nameProp == nullptr) {
                g_warning("SettingsDescription::data tag missing name in buttonConfig!");
                continue;
            }
            name = reinterpret_cast<const char*>(nameProp);
            xmlFree(nameProp);

            for (xmlNodePtr attr = cur->children; attr != nullptr; attr = attr->next) {
                if (xmlStrcmp(attr->name, reinterpret_cast<const xmlChar*>("attribute")) == 0) {
                    xmlChar* attrName = xmlGetProp(attr, reinterpret_cast<const xmlChar*>("name"));
                    xmlChar* value = xmlGetProp(attr, reinterpret_cast<const xmlChar*>("value"));
                    xmlChar* type = xmlGetProp(attr, reinterpret_cast<const xmlChar*>("type"));

                    if (attrName == nullptr || value == nullptr || type == nullptr) {
                        g_warning("SettingsDescription::attribute tag in deviceClasses for device '%s' is missing "
                                  "properties!",
                                  name.c_str());
                        continue;
                    }

                    if (xmlStrcmp(attrName, reinterpret_cast<const xmlChar*>("deviceClass")) == 0 &&
                        xmlStrcmp(type, reinterpret_cast<const xmlChar*>("int")) == 0) {
                        deviceClass = (int)g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
                        continue;
                    }
                    if (xmlStrcmp(attrName, reinterpret_cast<const xmlChar*>("deviceSource")) == 0 &&
                        xmlStrcmp(type, reinterpret_cast<const xmlChar*>("int")) == 0) {
                        deviceSource = (int)g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
                        continue;
                    }

                    if (xmlStrcmp(type, reinterpret_cast<const xmlChar*>("int")) != 0) {
                        g_warning("SettingsDescription::attribute '%s' in deviceClasses for device '%s' has non int "
                                  "value!",
                                  reinterpret_cast<const char*>(attrName), name.c_str());
                        continue;
                    }
                    g_warning("SettingsDescription::Unknown attribute '%s' in deviceClasses for device '%s'!",
                              reinterpret_cast<const char*>(attrName), name.c_str());
                } else {
                    g_warning("SettingsDescription::Unexpected xml tag '%s' in deviceClasses for device '%s'!",
                              reinterpret_cast<const char*>(cur->name), name.c_str());
                }
            }

            if (!(deviceClass.has_value() && deviceSource.has_value())) {
                g_warning("SettingsDescription::device '%s' in deviceClasses is missing attributes!", name.c_str());
                continue;
            }

            var.emplace(name, std::make_pair(static_cast<InputDeviceTypeOption>(deviceClass.value()),
                                             static_cast<GdkInputSource>(deviceSource.value())));

        } else {
            g_warning("SettingsDescription::Unexpected xml tag '%s' in buttonConfig!",
                      reinterpret_cast<const char*>(cur->name));
        }
    }
    return true;
}
bool importSElement(xmlNodePtr node, SElement& var) {
    for (xmlNodePtr cur = node->children; cur != nullptr; cur = cur->next) {
        if (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("data")) == 0) {
            xmlChar* name = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("name"));
            if (name == nullptr) {
                g_warning("SettingsDescription::data node for SElement import is missing name property");
            }
            importSElement(cur, var.child(reinterpret_cast<const char*>(name)));
            xmlFree(name);
        } else if (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("attribute")) == 0) {
            xmlChar* name = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("name"));
            xmlChar* value = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("value"));
            xmlChar* type = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("type"));

            if (!(name == nullptr || value == nullptr || type == nullptr)) {
                std::string typeString = reinterpret_cast<const char*>(type);

                if (typeString == "int") {
                    var.setInt(reinterpret_cast<const char*>(name),
                               static_cast<int>(g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10)));
                } else if (typeString == "double") {
                    var.setDouble(reinterpret_cast<const char*>(name),
                                  g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr));
                } else if (typeString == "hex") {
                    var.setIntHex(reinterpret_cast<const char*>(name),
                                  static_cast<int>(g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 16)));
                } else if (typeString == "string") {
                    var.setString(reinterpret_cast<const char*>(name), reinterpret_cast<const char*>(value));
                } else if (typeString == "boolean") {
                    var.setBool(reinterpret_cast<const char*>(name),
                                strcmp(reinterpret_cast<const char*>(value), "true") == 0);
                } else {
                    g_warning("SettingsDescription::Unknown datatype: %s\n", typeString.c_str());
                }

            } else {
                g_warning("SettingsDescription::\n");
            }

            xmlFree(name);
            xmlFree(type);
            xmlFree(value);
        } else {
            g_warning("SettingsDescription::Unknown XML node: %s\n", cur->name);
        }
    }
    return true;
}

// Declarations of export functions
xmlNodePtr exportStringProperty(xmlNodePtr node, std::string name, std::string value) {
    xmlNodePtr xmlNode = xmlNewChild(node, nullptr, reinterpret_cast<const xmlChar*>("property"), nullptr);
    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>(name.c_str()));
    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"), reinterpret_cast<const xmlChar*>(value.c_str()));
    return xmlNode;
}
xmlNodePtr exportPathProperty(xmlNodePtr node, std::string name, fs::path value) {
    return exportStringProperty(node, name, value.empty() ? "" : value.u8string().c_str());
}
xmlNodePtr exportBoolProperty(xmlNodePtr node, std::string name, bool value) {
    return exportStringProperty(node, name, value ? "true" : "false");
}
xmlNodePtr exportDoubleProperty(xmlNodePtr node, std::string name, double value) {
    char text[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(text, G_ASCII_DTOSTR_BUF_SIZE, value);
    return exportStringProperty(node, name, text);
}
xmlNodePtr exportIntProperty(xmlNodePtr node, std::string name, int value) {
    return exportStringProperty(node, name, std::to_string(value));
}
xmlNodePtr exportUintProperty(xmlNodePtr node, std::string name, uint value) {
    return exportStringProperty(node, name, std::to_string(value));
}
xmlNodePtr exportColorProperty(xmlNodePtr node, std::string name, Color value) {
    return exportUintProperty(node, name, u_int32_t(value));
}
xmlNodePtr exportULintProperty(xmlNodePtr node, std::string name, size_t value) {
    return exportStringProperty(node, name, std::to_string(value));
}
xmlNodePtr exportFont(xmlNodePtr node, std::string name, XojFont value) {
    xmlNodePtr xmlNode = xmlNewChild(node, nullptr, reinterpret_cast<const xmlChar*>("property"), nullptr);
    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("font"));
    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("font"),
               reinterpret_cast<const xmlChar*>(value.getName().c_str()));
    char text[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(text, G_ASCII_DTOSTR_BUF_SIZE, value.getSize());
    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("size"), reinterpret_cast<const xmlChar*>(text));
    return xmlNode;
}
xmlNodePtr exportSidebarNumbering(xmlNodePtr node, std::string name, SidebarNumberingStyle value) {
    return exportIntProperty(node, name, static_cast<int>(value));
}
xmlNodePtr exportStylusCursorType(xmlNodePtr node, std::string name, StylusCursorType value) {
    return exportStringProperty(node, name, stylusCursorTypeToString(value));
}
xmlNodePtr exportEraserVisibility(xmlNodePtr node, std::string name, EraserVisibility value) {
    return exportStringProperty(node, name, eraserVisibilityToString(value));
}
xmlNodePtr exportIconTheme(xmlNodePtr node, std::string name, IconTheme value) {
    return exportStringProperty(node, name, iconThemeToString(value));
}
xmlNodePtr exportViewMode(xmlNodePtr node, std::string name, ViewMode value) {
    return exportStringProperty(node, name, viewModeToSettingsString(value));
}
xmlNodePtr exportScrollbarHideType(xmlNodePtr node, std::string name, ScrollbarHideType value) {
    std::string val = "none";
    switch (value) {
        case SCROLLBAR_HIDE_BOTH:
            val = "both";
            break;
        case SCROLLBAR_HIDE_HORIZONTAL:
            val = "horizontal";
            break;
        case SCROLLBAR_HIDE_VERTICAL:
            val = "vertical";
            break;
        case SCROLLBAR_HIDE_NONE:
            break;
    }
    return exportStringProperty(node, name, val);
}
xmlNodePtr exportPADeviceIndex(xmlNodePtr node, std::string name, PaDeviceIndex value) {
    return exportIntProperty(node, name, static_cast<int>(value));
}
xmlNodePtr exportEmptyLastPageAppendType(xmlNodePtr node, std::string name, EmptyLastPageAppendType value) {
    return exportStringProperty(node, name, emptyLastPageAppendToString(value));
}
xmlNodePtr exportLatexSettings(xmlNodePtr node, std::string name, LatexSettings value) {
    xmlNodePtr xmlNode = exportBoolProperty(node, "latexSettings.autoCheckDependencies", value.autoCheckDependencies);
    xmlNode = exportStringProperty(node, "latexSettings.defaultText", value.defaultText);
    xmlNode = exportStringProperty(node, "latexSettings.globalTemplatePath", value.globalTemplatePath.string());
    xmlNode = exportStringProperty(node, "latexSettings.genCmd", value.genCmd);
    xmlNode = exportStringProperty(node, "latexSettings.sourceViewThemeId", value.sourceViewThemeId);
    xmlNode = exportStringProperty(node, "latexSettings.editorFont", value.editorFont.asString());
    xmlNode = exportBoolProperty(node, "latexSettings.useCustomEditorFont", value.useCustomEditorFont);
    xmlNode = exportBoolProperty(node, "latexSettings.editorWordWrap", value.editorWordWrap);
    xmlNode = exportBoolProperty(node, "latexSettings.sourceViewAutoIndent", value.sourceViewAutoIndent);
    xmlNode = exportBoolProperty(node, "latexSettings.sourceViewSyntaxHighlight", value.sourceViewSyntaxHighlight);
    xmlNode = exportBoolProperty(node, "latexSettings.sourceViewShowLineNumbers", value.sourceViewShowLineNumbers);
    return xmlNode;
}
xmlNodePtr exportStrokeAveragingMethod(xmlNodePtr node, std::string name, StrokeStabilizer::AveragingMethod value) {
    return exportIntProperty(node, name, static_cast<int>(value));
}
xmlNodePtr exportStrokePreprocessor(xmlNodePtr node, std::string name, StrokeStabilizer::Preprocessor value) {
    return exportIntProperty(node, name, static_cast<int>(value));
}
xmlNodePtr exportButtonConfig(xmlNodePtr node, std::string name,
                              const std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& value) {
    xmlNodePtr xmlNodeButtonConfig = xmlNewChild(node, nullptr, reinterpret_cast<const xmlChar*>("data"), nullptr);
    xmlSetProp(xmlNodeButtonConfig, reinterpret_cast<const xmlChar*>("name"),
               reinterpret_cast<const xmlChar*>(name.c_str()));
    for (int i = 0; i < BUTTON_COUNT; i++) {
        const auto& cfg = value[i];
        xmlNodePtr btnNode =
                xmlNewChild(xmlNodeButtonConfig, nullptr, reinterpret_cast<const xmlChar*>("data"), nullptr);
        xmlSetProp(btnNode, reinterpret_cast<const xmlChar*>("name"),
                   reinterpret_cast<const xmlChar*>(buttonToString(static_cast<Button>(i))));

        // save the tool type first
        ToolType const tool = cfg->action;
        xmlNodePtr xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("tool"));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("string"));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                   reinterpret_cast<const xmlChar*>(toolTypeToString(tool).c_str()));

        // if tool is pen save stroke type next
        if (tool == TOOL_PEN) {
            xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"),
                       reinterpret_cast<const xmlChar*>("strokeType"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("string"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                       reinterpret_cast<const xmlChar*>(strokeTypeToString(cfg->strokeType).c_str()));
        }

        // if tool is pen or highlighter save drawing type next
        if (tool == TOOL_PEN || tool == TOOL_HIGHLIGHTER) {
            xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"),
                       reinterpret_cast<const xmlChar*>("drawingType"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("string"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                       reinterpret_cast<const xmlChar*>(drawingTypeToString(cfg->drawingType).c_str()));
        }

        // if tool is pen, highlighter or eraser save size next
        if (tool == TOOL_PEN || tool == TOOL_HIGHLIGHTER || tool == TOOL_ERASER) {
            xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("size"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("string"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                       reinterpret_cast<const xmlChar*>(toolSizeToString(cfg->size).c_str()));
        }

        // if tool is pen, highlighter or text save color next
        if (tool == TOOL_PEN || tool == TOOL_HIGHLIGHTER || tool == TOOL_TEXT) {
            xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
            char* color = g_strdup_printf("%08x", uint32_t(cfg->color));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("color"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("hex"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"), reinterpret_cast<const xmlChar*>(color));
            g_free(color);
        }

        // if tool is eraser save eraser mode next
        if (tool == TOOL_ERASER) {
            xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"),
                       reinterpret_cast<const xmlChar*>("eraserMode"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("string"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                       reinterpret_cast<const xmlChar*>(eraserTypeToString(cfg->eraserMode).c_str()));
        }

        // if the button is touch device save touch device specific settings
        if (i == static_cast<int>(BUTTON_TOUCH)) {
            xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("device"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("string"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                       reinterpret_cast<const xmlChar*>(cfg->device.c_str()));

            xmlNode = xmlNewChild(btnNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"),
                       reinterpret_cast<const xmlChar*>("disableDrawing"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("boolean"));
            xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                       reinterpret_cast<const xmlChar*>(cfg->disableDrawing ? "true" : "false"));
        }
    }
    return xmlNodeButtonConfig;
}
xmlNodePtr exportDeviceClasses(xmlNodePtr node, std::string name,
                               const std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& value) {
    xmlNodePtr xmlNodeDeviceClasses = xmlNewChild(node, nullptr, reinterpret_cast<const xmlChar*>("data"), nullptr);
    xmlSetProp(xmlNodeDeviceClasses, reinterpret_cast<const xmlChar*>("name"),
               reinterpret_cast<const xmlChar*>(name.c_str()));
    for (auto& device: value) {
        xmlNodePtr devNode =
                xmlNewChild(xmlNodeDeviceClasses, nullptr, reinterpret_cast<const xmlChar*>("data"), nullptr);
        xmlSetProp(devNode, reinterpret_cast<const xmlChar*>("name"),
                   reinterpret_cast<const xmlChar*>(device.first.c_str()));

        xmlNodePtr xmlNode = xmlNewChild(devNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("deviceClass"));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("int"));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                   reinterpret_cast<const xmlChar*>(std::to_string(static_cast<int>(device.second.first)).c_str()));

        xmlNode = xmlNewChild(devNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("deviceSource"));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>("int"));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                   reinterpret_cast<const xmlChar*>(std::to_string(static_cast<int>(device.second.second)).c_str()));
    }
    return xmlNodeDeviceClasses;
}
xmlNodePtr exportSElement(xmlNodePtr node, std::string name, const SElement& value) {
    xmlNodePtr xmlDataNode = xmlNewChild(node, nullptr, reinterpret_cast<const xmlChar*>("data"), nullptr);
    xmlSetProp(xmlDataNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>(name.c_str()));
    for (auto const& [aname, attrib]: value.attributes()) {
        std::string typeStr;
        std::string valueStr;

        if (attrib.type == ATTRIBUTE_TYPE_BOOLEAN) {
            typeStr = "boolean";
            valueStr = attrib.iValue ? "true" : "false";
        } else if (attrib.type == ATTRIBUTE_TYPE_INT) {
            typeStr = "int";
            valueStr = std::to_string(attrib.iValue);
        } else if (attrib.type == ATTRIBUTE_TYPE_DOUBLE) {
            typeStr = "double";

            char text[G_ASCII_DTOSTR_BUF_SIZE];
            g_ascii_dtostr(text, G_ASCII_DTOSTR_BUF_SIZE, attrib.dValue);
            valueStr = text;
        } else if (attrib.type == ATTRIBUTE_TYPE_INT_HEX) {
            typeStr = "hex";

            char* tmp = g_strdup_printf("%06x", attrib.iValue);
            valueStr = tmp;
            g_free(tmp);
        } else if (attrib.type == ATTRIBUTE_TYPE_STRING) {
            typeStr = "string";
            valueStr = attrib.sValue;
        } else {
            g_warning("Unknown type in SElement setting '%s'!", name.c_str());
            continue;
        }

        xmlNodePtr xmlNode = xmlNewChild(xmlDataNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>(aname.c_str()));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("type"),
                   reinterpret_cast<const xmlChar*>(typeStr.c_str()));
        xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"),
                   reinterpret_cast<const xmlChar*>(valueStr.c_str()));

        if (!attrib.comment.empty()) {
            xmlNodePtr com = xmlNewComment(reinterpret_cast<const xmlChar*>(attrib.comment.c_str()));
            xmlAddPrevSibling(xmlDataNode, com);
        }
    }
    for (auto const& p: value.children()) { exportSElement(xmlDataNode, p.first, p.second); }
    return xmlDataNode;
}
