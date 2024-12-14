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

class ScrollHandler;
class Control;
class ActionDatabase;
class TextEditor;
class SplineHandler;
class BaseShapeHandler;
class EditSelection;
class GeometryToolInputHandler;
class VerticalTool;

struct ActionKey {
    ActionKey(Action a): action(a) {}
    ActionKey(Action a, uint64_t p): action(a), parameter(p) {}
    Action action;
    std::optional<uint64_t> parameter{};  // We only have shortcuts for Actions with enum parameters (if any)

    constexpr bool operator==(const ActionKey& o) const { return action == o.action && parameter == o.parameter; }
};

template <>
struct std::hash<ActionKey> {
    constexpr static uint64_t NULLOPT = 0xffffffffafffffff;  // Arbitrary but not a plausible parameter value
    constexpr std::size_t operator()(const ActionKey& k) const noexcept {
        return static_cast<size_t>(k.action) ^ (static_cast<size_t>(k.parameter.value_or(NULLOPT)) << 1);
    }
};

class ShortcutConfiguration {
public:
    ShortcutConfiguration();

    inline const auto& getActionsShortcuts() const { return actionsShortcuts; }
    inline const auto& getScrollShortcuts() const { return scrollShortcuts; }
    inline const auto& getOtherShortcuts() const { return otherShortcuts; }
    inline const auto& getShunt() const { return shuntGtkDefaultBindings; }

private:
    std::unordered_map<ActionKey, std::vector<Shortcut>> actionsShortcuts;

    KeyBindingsGroup<ScrollHandler> scrollShortcuts;
    KeyBindingsGroup<Control> otherShortcuts;
    // KeyBindingsGroup<TextEditor> textEditorShortcuts;
    // KeyBindingsGroup<SplineHandler> splineHandlerShortcuts;
    // KeyBindingsGroup<BaseShapeHandler> shapeHandlerShortcuts;
    // KeyBindingsGroup<EditSelection> selectionShortcuts;
    // KeyBindingsGroup<GeometryToolInputHandler> geometryToolsShortcuts;
    // KeyBindingsGroup<VerticalTool> verticalToolsShortcuts;

    /**
     * GTK defines default bindings (arrow keys, Page_Up/Down, Tab and so on). We want to grab those before GTK does.
     * The key bindings in this group will be caught in XournalView and forwarded to the Action shortcut management.
     *
     * This group is generated from
     *  1- A (static) list of GTK bindings to overrule
     *  2- The above actionsShortcuts
     */
    KeyBindingsGroup<ActionDatabase> shuntGtkDefaultBindings;
};
