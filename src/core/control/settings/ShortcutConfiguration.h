/*
 * Xournal++
 *
 * Configuration data for shortcuts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <optional>
#include <vector>

#include "control/KeyBindingsGroup.h"
#include "enums/Action.enum.h"

class ActionDatabase;
class TextEditor;
class SplineHandler;
class BaseShapeHandler;
class EditSelection;
class GeometryToolInputHandler;
class VerticalTool;

struct ShortcutConfigurationKey {
    ShortcutConfigurationKey(Action a): action(a) {}
    ShortcutConfigurationKey(Action a, uint64_t p): action(a), parameter(p) {}
    Action action;
    std::optional<uint64_t> parameter{};  // We only have shortcuts for Actions with enum parameters (if any)

    constexpr bool operator==(const ShortcutConfigurationKey& o) const { return action == o.action && parameter == o.parameter ; }
};

template<>
struct std::hash<ShortcutConfigurationKey> {
    constexpr static uint64_t NULLOPT = 0xffffffffafffffff; // Arbitrary but not a plausible parameter value
    constexpr std::size_t operator()(const ShortcutConfigurationKey& k) const noexcept {
        return static_cast<size_t>(k.action) ^ (static_cast<size_t>(k.parameter.value_or(NULLOPT)) << 1);
    }
};

class ShortcutConfiguration {
public:
    ShortcutConfiguration();

    inline const std::unordered_map<ShortcutConfigurationKey, std::vector<Shortcut>>& getActionsShortcuts() const { return actionsShortcuts; }

private:
    std::unordered_map<ShortcutConfigurationKey, std::vector<Shortcut>> actionsShortcuts;
    // KeyBindingsGroup<TextEditor> textEditorShortcuts;
    // KeyBindingsGroup<SplineHandler> splineHandlerShortcuts;
    // KeyBindingsGroup<BaseShapeHandler> shapeHandlerShortcuts;
    // KeyBindingsGroup<EditSelection> selectionShortcuts;
    // KeyBindingsGroup<GeometryToolInputHandler> geometryToolsShortcuts;
    // KeyBindingsGroup<VerticalTool> verticalToolsShortcuts;
};
