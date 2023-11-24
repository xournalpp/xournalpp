/*
 * Xournal++
 *
 * File containing importers and exporters for the settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <libxml/tree.h>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "control/ToolEnums.h"
#include "control/tools/StrokeStabilizerEnum.h"
#include "model/Font.h"
#include "util/Color.h"

#include "ButtonConfig.h"
#include "LatexSettings.h"
#include "SettingsEnums.h"
#include "ViewModes.h"
#include "filesystem.h"

class SElement;

// Definition of import function template
template <typename T>
bool importProperty(xmlNodePtr node, T& var);

template <>
bool importProperty(xmlNodePtr node, std::string& var);
template <>
bool importProperty(xmlNodePtr node, fs::path& var);
template <>
bool importProperty(xmlNodePtr node, bool& var);
template <>
bool importProperty(xmlNodePtr node, double& var);
template <>
bool importProperty(xmlNodePtr node, int& var);
template <>
bool importProperty(xmlNodePtr node, uint& var);
template <>
bool importProperty(xmlNodePtr node, Color& var);
template <>
bool importProperty(xmlNodePtr node, size_t& var);
template <>
bool importProperty(xmlNodePtr node, ViewMode& var);
template <>
bool importProperty(xmlNodePtr node, SElement& var);


// exportProp function
xmlNodePtr exportProp(xmlNodePtr parent, const char* name, const char* value);


// Definition of export function template
template <typename T>
xmlNodePtr exportProperty(xmlNodePtr parent, std::string name, T value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const std::string& value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const fs::path& value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, bool value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, double value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, int value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, uint value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, Color value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, size_t value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, ViewMode value);
template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const SElement& value);

// Definitions of setting specific import functions
bool importButtonConfig(xmlNodePtr node, std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& var);
bool importDeviceClasses(xmlNodePtr node, std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& var);
bool importSidebarNumberingStyle(xmlNodePtr node, SidebarNumberingStyle& var);
bool importStylusCursorType(xmlNodePtr node, StylusCursorType& var);
bool importEraserVisibility(xmlNodePtr node, EraserVisibility& var);
bool importIconTheme(xmlNodePtr node, IconTheme& var);
bool importScrollbarHideType(xmlNodePtr node, ScrollbarHideType& var);
bool importEmptyLastPageAppendType(xmlNodePtr node, EmptyLastPageAppendType& var);
bool importLatexSettings(xmlNodePtr node, LatexSettings& var);
bool importAveragingMethod(xmlNodePtr node, StrokeStabilizer::AveragingMethod& var);
bool importPreprocessor(xmlNodePtr node, StrokeStabilizer::Preprocessor& var);
bool importFont(xmlNodePtr node, XojFont& var);

// Definitions of setting specific export functions
xmlNodePtr exportButtonConfig(xmlNodePtr node, std::string name,
                              const std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& value);
xmlNodePtr exportDeviceClasses(xmlNodePtr node, std::string name,
                               const std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& value);
xmlNodePtr exportScrollbarHideType(xmlNodePtr node, std::string name, ScrollbarHideType value);
xmlNodePtr exportLatexSettings(xmlNodePtr node, std::string name, const LatexSettings& value);
xmlNodePtr exportFont(xmlNodePtr node, std::string name, const XojFont& value);
