#include "ShortcutManager.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include <glib.h>
#include <gtk/gtk.h>

#include "control/Control.h"
#include "control/actions/ActionDatabase.h"
#include "control/actions/ActionProperties.h"
#include "control/settings/Settings.h"
#include "control/ToolEnums.h"
#include "enums/Action.enum.h"

ShortcutManager::ShortcutManager(Control* control): control(control) {}

// ==================== Helper Functions ====================

/**
 * Helper function to set GTK accelerator for an action.
 * Uses a static array to ensure the pointer remains valid for GTK.
 * 
 * @param app GTKApplication instance
 * @param action Action name to set accelerator for
 * @param accelerator Accelerator string (empty to clear/disable)
 */
static void setGtkAccelerator(GtkApplication* app, const char* action, const std::string& accelerator) {
    if (!app) return;
    if (accelerator.empty()) {
        gtk_application_set_accels_for_action(app, action, nullptr);
    } else {
        // GTK requires a null-terminated array of accelerator strings
        // Use static to ensure the pointer remains valid
        static const char* accels[2] = {nullptr, nullptr};
        accels[0] = accelerator.c_str();
        gtk_application_set_accels_for_action(app, action, accels);
    }
}

// ==================== Template Helpers for Default Shortcut Registration ====================

/**
 * Helper to convert action name to human-readable display name.
 * E.g. "tool-pen-size" -> "Pen Size", "zoom-in" -> "Zoom In"
 */
static std::string actionToDisplayName(const std::string& actionName) {
    // Remove namespace prefix (win. or app.)
    size_t dotPos = actionName.find('.');
    std::string name = (dotPos != std::string::npos) ? actionName.substr(dotPos + 1) : actionName;
    
    // Replace hyphens with spaces and capitalize words
    std::string result;
    bool lastWasHyphen = true;
    for (char c : name) {
        if (c == '-') {
            result += ' ';
            lastWasHyphen = true;
        } else {
            if (lastWasHyphen && c >= 'a' && c <= 'z') {
                result += c - 32;  // Capitalize
            } else {
                result += c;
            }
            lastWasHyphen = false;
        }
    }
    return result;
}

/**
 * Get the category for an action based on its name.
 * Categories: File, Edit, View, Page, Layer, Tools, Zoom, Navigation, etc.
 */
static std::string getActionCategory(const std::string& actionName) {
    size_t dotPos = actionName.find('.');
    std::string action = (dotPos != std::string::npos) ? actionName.substr(dotPos + 1) : actionName;
    
    // File operations
    if (action == "new-file" || action == "open" || action == "save" || action == "save-as" ||
        action == "export-as" || action == "export-as-pdf" || action == "print" || action == "quit" ||
        action == "annotate-pdf" || action == "append-new-pdf-pages")
        return _("File");
    
    // Edit operations
    if (action == "undo" || action == "redo" || action == "cut" || action == "copy" || action == "paste" ||
        action == "delete" || action == "select-all" || action == "search")
        return _("Edit");
    
    // View operations
    if (action == "fullscreen" || action == "presentation-mode" || action == "show-sidebar" ||
        action == "show-toolbar" || action == "show-menubar" || action == "paired-pages-mode" ||
        action == "paired-pages-offset" || action == "rotation-snapping" || action == "grid-snapping" ||
        action == "toggle-touch-drawing" || action == "position-highlighting" ||
        action == "set-layout-vertical" || action == "set-layout-right-to-left" ||
        action == "set-layout-bottom-to-top" || action == "set-columns-or-rows")
        return _("View");
    
    // Page operations
    if (action == "goto-first" || action == "goto-previous" || action == "goto-next" ||
        action == "goto-last" || action == "goto-page" || action == "goto-next-annotated-page" ||
        action == "goto-previous-annotated-page" || action == "new-page-before" ||
        action == "new-page-after" || action == "new-page-at-end" || action == "duplicate-page" ||
        action == "delete-page" || action == "move-page-towards-beginning" ||
        action == "move-page-towards-end" || action == "paper-format" ||
        action == "paper-background-color" || action == "configure-page-template")
        return _("Page");
    
    // Layer operations
    if (action == "layer-new" || action == "layer-delete" || action == "layer-show-all" ||
        action == "layer-hide-all" || action == "layer-merge-down" || action == "layer-copy" ||
        action == "layer-move-up" || action == "layer-move-down" || action == "layer-goto-next" ||
        action == "layer-goto-previous" || action == "layer-goto-top" || action == "layer-active" ||
        action == "layer-rename")
        return _("Layer");
    
    // Tool operations
    if (action == "select-tool" || action == "select-default-tool" ||
        action == "tool-pen-size" || action == "tool-pen-line-style" || action == "tool-pen-fill" ||
        action == "tool-pen-fill-opacity" || action == "tool-eraser-size" || action == "tool-eraser-type" ||
        action == "tool-highlighter-size" || action == "tool-highlighter-fill" ||
        action == "tool-highlighter-fill-opacity" || action == "tool-color" || action == "tool-size" ||
        action == "tool-fill" || action == "tool-fill-opacity" ||
        action == "tool-draw-shape-recognizer" || action == "tool-draw-rectangle" ||
        action == "tool-draw-ellipse" || action == "tool-draw-arrow" ||
        action == "tool-draw-double-arrow" || action == "tool-draw-line" ||
        action == "tool-draw-spline" || action == "tool-draw-coordinate-system" ||
        action == "compass" || action == "setsquare")
        return _("Tools");
    
    // Zoom operations
    if (action == "zoom-in" || action == "zoom-out" || action == "zoom-100" || action == "zoom-fit" || action == "zoom")
        return _("Zoom");
    
    // Navigation
    if (action == "navigate-back" || action == "navigate-forward")
        return _("Navigation");
    
    // Audio
    if (action == "audio-record" || action == "audio-playback" || action == "audio-stop" ||
        action == "audio-seek-forwards" || action == "audio-seek-backwards")
        return _("Audio");
    
    // Special
    if (action == "preferences" || action == "customize-shortcuts" || action == "customize-toolbar" ||
        action == "manage-toolbar" || action == "plugin-manager" || action == "help" || action == "about" ||
        action == "font" || action == "select-font" || action == "select-color" || action == "tex" ||
        action == "arrange-selection-order" || action == "move-selection-layer-up" ||
        action == "move-selection-layer-down")
        return _("Special");
    
    return _("General");
}

/**
 * Get translatable description for an action.
 * Returns a human-readable description of what the action does.
 * Supports both English and Chinese based on locale.
 */
static std::string getActionDescription(const std::string& actionName) {
    // Extract action part without namespace
    size_t dotPos = actionName.find('.');
    std::string action = (dotPos != std::string::npos) ? actionName.substr(dotPos + 1) : actionName;
    
    // Map of action -> description (English, Chinese)
    
    if (action == "new-file") return _("Create a new document");
    if (action == "open") return _("Open an existing document");
    if (action == "annotate-pdf") return _("Annotate an existing PDF");
    if (action == "save") return _("Save the current document");
    if (action == "save-as") return _("Save with a new name");
    if (action == "export-as-pdf") return _("Export document as PDF");
    if (action == "export-as") return _("Export in various formats");
    if (action == "print") return _("Print the document");
    if (action == "quit") return _("Exit the application");
    if (action == "undo") return _("Undo the last action");
    if (action == "redo") return _("Redo the last undone action");
    if (action == "cut") return _("Cut selection to clipboard");
    if (action == "copy") return _("Copy selection to clipboard");
    if (action == "paste") return _("Paste from clipboard");
    if (action == "search") return _("Search in document");
    if (action == "select-all") return _("Select all content");
    if (action == "delete") return _("Delete selection");
    if (action == "zoom-in") return _("Increase zoom level");
    if (action == "zoom-out") return _("Decrease zoom level");
    if (action == "zoom-100") return _("Reset zoom to 100%");
    if (action == "zoom-fit") return _("Fit content to window");
    if (action == "fullscreen") return _("Toggle fullscreen mode");
    if (action == "show-sidebar") return _("Toggle sidebar visibility");
    if (action == "show-toolbar") return _("Toggle toolbar visibility");
    if (action == "show-menubar") return _("Toggle menubar visibility");
    if (action == "preferences") return _("Open settings dialog");
    if (action == "customize-shortcuts") return _("Customize keyboard shortcuts");
    if (action == "goto-first") return _("Go to first page");
    if (action == "goto-previous") return _("Go to previous page");
    if (action == "goto-next") return _("Go to next page");
    if (action == "goto-last") return _("Go to last page");
    if (action == "new-page-before") return _("Insert page before current");
    if (action == "new-page-after") return _("Insert page after current");
    if (action == "new-page-at-end") return _("Append new page at end");
    if (action == "duplicate-page") return _("Duplicate current page");
    if (action == "delete-page") return _("Delete current page");
    if (action == "select-tool") return _("Select tool");
    if (action == "select-default-tool") return _("Select default tool");
    if (action == "tool-pen-size") return _("Change pen thickness");
    if (action == "tool-pen-line-style") return _("Change line style");
    if (action == "tool-pen-fill") return _("Toggle pen fill");
    if (action == "tool-pen-fill-opacity") return _("Pen fill opacity");
    if (action == "tool-eraser-size") return _("Change eraser size");
    if (action == "tool-eraser-type") return _("Change eraser type");
    if (action == "tool-highlighter-size") return _("Change highlighter size");
    if (action == "tool-highlighter-fill") return _("Toggle highlighter fill");
    if (action == "tool-highlighter-fill-opacity") return _("Highlighter opacity");
    if (action == "tool-draw-shape-recognizer") return _("Draw with shape recognition");
    if (action == "tool-draw-rectangle") return _("Draw rectangle");
    if (action == "tool-draw-ellipse") return _("Draw ellipse");
    if (action == "tool-draw-arrow") return _("Draw arrow");
    if (action == "tool-draw-double-arrow") return _("Draw double arrow");
    if (action == "tool-draw-line") return _("Draw straight line");
    if (action == "tool-draw-spline") return _("Draw spline");
    if (action == "tool-draw-coordinate-system") return _("Draw coordinate system");
    if (action == "tool-color") return _("Change drawing color");
    if (action == "tool-size") return _("Change tool size");
    if (action == "tool-fill") return _("Toggle fill");
    if (action == "tool-fill-opacity") return _("Fill opacity");
    if (action == "layer-show-all") return _("Show all layers");
    if (action == "layer-hide-all") return _("Hide all layers");
    if (action == "layer-new") return _("Create new layer");
    if (action == "layer-delete") return _("Delete current layer");
    if (action == "layer-merge-down") return _("Merge with layer below");
    if (action == "layer-copy") return _("Copy layer");
    if (action == "layer-move-up") return _("Move layer up");
    if (action == "layer-move-down") return _("Move layer down");
    if (action == "layer-goto-next") return _("Go to next layer");
    if (action == "layer-goto-previous") return _("Go to previous layer");
    if (action == "layer-goto-top") return _("Go to top layer");
    if (action == "layer-active") return _("Toggle layer visibility");
    if (action == "layer-rename") return _("Rename layer");
    if (action == "presentation-mode") return _("Toggle presentation mode");
    if (action == "paired-pages-mode") return _("Toggle paired pages");
    if (action == "paired-pages-offset") return _("Toggle paired pages offset");
    if (action == "rotation-snapping") return _("Toggle rotation snapping");
    if (action == "grid-snapping") return _("Toggle grid snapping");
    if (action == "toggle-touch-drawing") return _("Toggle touch drawing");
    if (action == "position-highlighting") return _("Toggle position highlighting");
    if (action == "set-layout-vertical") return _("Set vertical layout");
    if (action == "set-layout-right-to-left") return _("Set right-to-left layout");
    if (action == "set-layout-bottom-to-top") return _("Set bottom-to-top layout");
    if (action == "set-columns-or-rows") return _("Set columns or rows");
    if (action == "paper-background-color") return _("Paper background color");
    if (action == "paper-format") return _("Paper format");
    if (action == "configure-page-template") return _("Configure page template");
    if (action == "font") return _("Select font");
    if (action == "select-font") return _("Select font");
    if (action == "tex") return _("Insert LaTeX");
    if (action == "compass") return _("Toggle compass tool");
    if (action == "setsquare") return _("Toggle set square tool");
    if (action == "plugin-manager") return _("Plugin manager");
    if (action == "help") return _("Show help");
    if (action == "about") return _("Show about dialog");
    if (action == "audio-record") return _("Record audio");
    if (action == "audio-playback") return _("Play/pause audio");
    if (action == "audio-stop") return _("Stop audio");
    if (action == "audio-seek-forwards") return _("Seek forwards");
    if (action == "audio-seek-backwards") return _("Seek backwards");
    if (action == "move-page-towards-beginning") return _("Move page towards beginning");
    if (action == "move-page-towards-end") return _("Move page towards end");
    if (action == "append-new-pdf-pages") return _("Append new PDF pages");
    if (action == "navigate-back") return _("Navigate back");
    if (action == "navigate-forward") return _("Navigate forward");
    if (action == "goto-page") return _("Go to page dialog");
    if (action == "goto-next-annotated-page") return _("Go to next annotated page");
    if (action == "goto-previous-annotated-page") return _("Go to previous annotated page");
    
    // Default: return the action name formatted nicely
    return actionToDisplayName(action);
}

/**
 * SFINAE helper to detect if an ActionProperties specialization has accelerators.
 * Matches if ActionProperties<a>::accelerators exists.
 */
template <Action a, class U = void>
struct HasAccelerators : std::false_type {};
template <Action a>
struct HasAccelerators<a, std::void_t<decltype(&ActionProperties<a>::accelerators)>> : std::true_type {};

/**
 * Helper to determine the action namespace prefix.
 * Most actions use "win." prefix, but some system actions use "app.".
 * Detected via ActionProperties<a>::app_namespace being std::true_type.
 */
template <Action a, class U = void>
struct ActionNamespaceHelper {
    static constexpr const char* NAMESPACE = "win.";  // Default namespace
};
template <Action a>
struct ActionNamespaceHelper<a, std::enable_if_t<std::is_same_v<typename ActionProperties<a>::app_namespace, std::true_type>, void>> {
    static constexpr const char* NAMESPACE = "app.";  // Application-level action
};

/**
 * Register a single action's shortcut if it has accelerators defined.
 * Only instantiated for actions that have the accelerators member.
 * Even if no default accelerator, registers with empty string so it shows in dialog.
 */
template <Action a>
typename std::enable_if_t<HasAccelerators<a>::value> registerSingleShortcut(ShortcutManager* sm) {
    constexpr const char* const* accels = ActionProperties<a>::accelerators;
    // Construct full action name with namespace prefix
    std::string baseActionName = ActionNamespaceHelper<a>::NAMESPACE;
    baseActionName += Action_toString(a);
    
    // Use human-readable display name and determine category
    std::string baseDisplayName = actionToDisplayName(baseActionName);
    std::string category = getActionCategory(baseActionName);
    
    // For SELECT_TOOL, expand into individual entries per tool type
    if constexpr (a == Action::SELECT_TOOL) {
        // toolNames[0] = "none", toolNames[1] = "pen", etc.
        // accels array has entries like {"<Ctrl><Shift>p", "<Ctrl><Shift>e", ...}
        for (int i = 1; i < TOOL_END_ENTRY; i++) {  // Start from 1, skip TOOL_NONE
            ToolType toolType = static_cast<ToolType>(i);
            std::string toolName(toolNames[static_cast<size_t>(i)]);
            std::string expandedAction = baseActionName + "(" + toolName + ")";
            std::string expandedDisplay = baseDisplayName + " (" + toolName + ")";
            
            // Get the accelerator for this tool - accels[0] is Ctrl+Shift+p, accels[1] is Ctrl+Shift+e, etc.
            // But the mapping isn't direct - we need to find the accel that matches this tool
            // For now, use the menu accelerators from mainmenubar.xml
            std::string accel;
            if (i == TOOL_PEN) accel = "<Ctrl><Shift>p";
            else if (i == TOOL_ERASER) accel = "<Ctrl><Shift>e";
            else if (i == TOOL_HIGHLIGHTER) accel = "<Ctrl><Shift>h";
            else if (i == TOOL_TEXT) accel = "<Ctrl><Shift>t";
            else if (i == TOOL_SELECT_PDF_TEXT_LINEAR) accel = "<Ctrl><Shift>w";
            else if (i == TOOL_SELECT_PDF_TEXT_RECT) accel = "<Ctrl><Shift>y";
            else if (i == TOOL_DRAW_RECT) accel = "<Ctrl>2";
            else if (i == TOOL_DRAW_ELLIPSE) accel = "<Ctrl>3";
            else if (i == TOOL_DRAW_ARROW) accel = "<Ctrl>4";
            else if (i == TOOL_DRAW_DOUBLE_ARROW) accel = "<Ctrl>5";
            else if (i == TOOL_DRAW_COORDINATE_SYSTEM) accel = "<Ctrl>6";
            else accel = "";  // No default for other tools
            
            // Create GVariant parameter for this tool
            GVariant* param = g_variant_new_uint32(static_cast<guint32>(i));
            sm->registerExpandedShortcut(expandedAction, baseActionName, category, expandedDisplay, accel, param);
        }
    } else {
        // Default behavior for non-expanded actions
        std::string defaultAccel = (accels && accels[0] != nullptr) ? accels[0] : "";
        sm->registerShortcut(baseActionName, category, baseDisplayName, defaultAccel);
    }
}

/**
 * Register actions even without accelerators - shows in dialog for user customization.
 * SFINAE ensures this is only instantiated when HasAccelerators<a> is false.
 */
template <Action a>
typename std::enable_if_t<!HasAccelerators<a>::value> registerSingleShortcut(ShortcutManager* sm) {
    // Register with empty accelerator so action shows in dialog for customization
    std::string actionName = ActionNamespaceHelper<a>::NAMESPACE;
    actionName += Action_toString(a);
    std::string displayName = actionToDisplayName(actionName);
    std::string category = getActionCategory(actionName);
    sm->registerShortcut(actionName, category, displayName, "");
}

/**
 * Iterate through all Action enum values and register shortcuts.
 * Uses parameter pack expansion with index_sequence.
 */
template <size_t... As>
void registerShortcutsImpl(ShortcutManager* sm, std::index_sequence<As...>) {
    // Expand and call registerSingleShortcut for each action index
    ((registerSingleShortcut<static_cast<Action>(As)>(sm)), ...);
}

// ==================== Public Methods ====================

void ShortcutManager::registerDefaultShortcuts() {
    // Register shortcuts for all actions that have accelerators defined
    registerShortcutsImpl(this, std::make_index_sequence<xoj::to_underlying(Action::ENUMERATOR_COUNT)>());
}

void ShortcutManager::registerShortcut(const std::string& action,
                                     const std::string& category,
                                     const std::string& displayName,
                                     const std::string& defaultAccel) {
    // Populate ShortcutInfo with all metadata
    ShortcutInfo info;
    info.action = action;
    info.category = category;
    info.displayName = displayName;
    info.description = getActionDescription(action);  // Get translatable description
    info.defaultAccel = defaultAccel;
    info.currentAccel = defaultAccel;
    info.enabled = !defaultAccel.empty();
    info.isDefault = true;
    
    // Store in both maps
    shortcutInfos_[action] = info;
    shortcuts_[action] = ShortcutConfig(defaultAccel, !defaultAccel.empty());
    categories_[category].push_back(action);
}

void ShortcutManager::registerShortcuts(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& shortcuts) {
    for (const auto& [action, category, displayName, defaultAccel] : shortcuts) {
        registerShortcut(action, category, displayName, defaultAccel);
    }
}

ShortcutConfig ShortcutManager::getShortcut(const std::string& action) const {
    auto it = shortcuts_.find(action);
    if (it != shortcuts_.end()) {
        return it->second;
    }
    return ShortcutConfig();
}

bool ShortcutManager::setShortcut(const std::string& action, const std::string& accelerator) {
    // Empty accelerator means disable the shortcut
    if (!accelerator.empty()) {
        if (!validateAccelerator(accelerator)) {
            return false;  // Invalid format
        }
        if (hasConflict(action, accelerator)) {
            return false;  // Conflicts with another shortcut
        }
    }
    
    // Update runtime configuration
    shortcuts_[action].accelerator = accelerator;
    shortcuts_[action].enabled = !accelerator.empty();
    
    // Update metadata if we have info for this action
    if (shortcutInfos_.find(action) != shortcutInfos_.end()) {
        shortcutInfos_[action].currentAccel = accelerator;
        shortcutInfos_[action].enabled = !accelerator.empty();
        shortcutInfos_[action].isDefault = 
            (accelerator == shortcutInfos_[action].defaultAccel);
    }
    
    // Sync with GTK accelerator map so changes take effect immediately
    GtkApplication* app = GTK_APPLICATION(gtk_window_get_application(control->getGtkWindow()));
    setGtkAccelerator(app, action.c_str(), accelerator);
    
    return true;
}

void ShortcutManager::registerExpandedShortcut(const std::string& expandedName,
                                              const std::string& baseName,
                                              const std::string& category,
                                              const std::string& displayName,
                                              const std::string& defaultAccel,
                                              GVariant* param) {
    // Store the mapping
    expandedToBase_[expandedName] = baseName;
    
    // Store the parameter (take ownership)
    expandedParams_[expandedName] = g_variant_ref_sink(param);
    
    // Register in normal maps
    registerShortcut(expandedName, category, displayName, defaultAccel);
    
    // Create virtual action for this expanded shortcut
    createVirtualAction(expandedName, baseName, param);
}

void ShortcutManager::createVirtualAction(const std::string& expandedName,
                                        const std::string& baseName,
                                        GVariant* param) {
    // Get the action database to find the original action's parameter type
    // We'll create a GSimpleAction that wraps the base action with a fixed parameter
    // When triggered, it will call the base action with our parameter
    
    // Create a new GSimpleAction with parameter type
    auto* action = g_simple_action_new(
        expandedName.c_str(),
        g_variant_get_type(param)
    );
    
    // Set up the callback
    // Use a static map to store lookup data for callbacks keyed by action name
    static std::map<std::string, std::pair<ShortcutManager*, std::string>> callbackMap;
    callbackMap[expandedName] = {this, baseName};
    
    // Callback retrieves ShortcutManager via static map lookup by action name
    g_signal_connect(action, "activate", G_CALLBACK(+[](GSimpleAction* sa, GVariant*, gpointer) {
        const char* name = g_action_get_name(G_ACTION(sa));
        auto it = callbackMap.find(name);
        if (it == callbackMap.end()) return;
        
        auto* sm = it->second.first;
        const std::string& baseName = it->second.second;
        
        auto paramIt = sm->expandedParams_.find(name);
        if (paramIt == sm->expandedParams_.end()) return;
        
        // Activate the base action with our stored parameter
        g_action_group_activate_action(
            G_ACTION_GROUP(gtk_window_get_application(sm->control->getGtkWindow())),
            baseName.c_str(),
            g_variant_ref(paramIt->second)
        );
    }), nullptr);
    
    // Add to action map so GTK can find it
    g_action_map_add_action(G_ACTION_MAP(gtk_window_get_application(control->getGtkWindow())), G_ACTION(action));
    
    // Register any existing shortcut for this expanded action
    auto shortcutIt = shortcuts_.find(expandedName);
    if (shortcutIt != shortcuts_.end() && shortcutIt->second.enabled && !shortcutIt->second.accelerator.empty()) {
        setGtkAccelerator(gtk_window_get_application(control->getGtkWindow()), expandedName.c_str(), shortcutIt->second.accelerator);
    }
}

void ShortcutManager::resetToDefault(const std::string& action) {
    auto it = shortcutInfos_.find(action);
    if (it != shortcutInfos_.end()) {
        // Reset to default values
        shortcuts_[action] = ShortcutConfig(it->second.defaultAccel, !it->second.defaultAccel.empty());
        it->second.currentAccel = it->second.defaultAccel;
        it->second.enabled = !it->second.defaultAccel.empty();
        it->second.isDefault = true;
        
        // Sync with GTK accelerator map
        GtkApplication* app = GTK_APPLICATION(gtk_window_get_application(control->getGtkWindow()));
        setGtkAccelerator(app, action.c_str(), it->second.defaultAccel);
    }
}

void ShortcutManager::resetAllToDefault() {
    GtkApplication* app = GTK_APPLICATION(gtk_window_get_application(control->getGtkWindow()));
    
    for (auto& [action, info] : shortcutInfos_) {
        // Reset to default values
        shortcuts_[action] = ShortcutConfig(info.defaultAccel, !info.defaultAccel.empty());
        info.currentAccel = info.defaultAccel;
        info.enabled = !info.defaultAccel.empty();
        info.isDefault = true;
        
        // Sync with GTK accelerator map
        setGtkAccelerator(app, action.c_str(), info.defaultAccel);
    }
}

bool ShortcutManager::trigger(guint keyval, GdkModifierType state) {
    // Mask out irrelevant modifiers (only keep the ones we care about)
    state = static_cast<GdkModifierType>(
        state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK | GDK_META_MASK));
    
    for (const auto& [action, config] : shortcuts_) {
        if (!config.enabled || config.accelerator.empty()) {
            continue;
        }
        
        auto parsed = parseAccelerator(config.accelerator);
        if (!parsed) {
            continue;
        }
        
        auto [targetKeyval, targetState] = *parsed;
        
        // Check if this key event matches this shortcut
        if (keyval == targetKeyval && state == targetState) {
            return true;
        }
    }
    return false;
}

void ShortcutManager::loadFromSettings() {
    auto* settings = control->getSettings();
    auto customAccels = settings->getCustomAccelerators();
    
    // Apply custom shortcuts from settings, overriding defaults
    for (const auto& [action, accel] : customAccels) {
        if (shortcuts_.find(action) != shortcuts_.end()) {
            shortcuts_[action].accelerator = accel;
            shortcuts_[action].enabled = !accel.empty();
            
            if (shortcutInfos_.find(action) != shortcutInfos_.end()) {
                shortcutInfos_[action].currentAccel = accel;
                shortcutInfos_[action].enabled = !accel.empty();
                shortcutInfos_[action].isDefault = (accel == shortcutInfos_[action].defaultAccel);
            }
        }
    }
}

void ShortcutManager::saveToSettings() {
    auto* settings = control->getSettings();
    settings->clearCustomAccelerators();
    
    // Only save shortcuts that differ from their defaults
    for (const auto& [action, config] : shortcuts_) {
        auto it = shortcutInfos_.find(action);
        if (it != shortcutInfos_.end() && !it->second.isDefault && config.enabled) {
            settings->setCustomAccelerator(action, config.accelerator);
        }
    }
    
    // Single save call for efficiency (batch operation)
    settings->save();
}

std::vector<ShortcutInfo> ShortcutManager::getAllShortcuts() const {
    std::vector<ShortcutInfo> result;
    for (const auto& [action, info] : shortcutInfos_) {
        ShortcutInfo i = info;
        // Merge in current runtime state
        auto it = shortcuts_.find(action);
        if (it != shortcuts_.end()) {
            i.currentAccel = it->second.accelerator;
            i.enabled = it->second.enabled;
        }
        result.push_back(i);
    }
    return result;
}

std::vector<std::string> ShortcutManager::getCategories() const {
    std::vector<std::string> result;
    for (const auto& [cat, _] : categories_) {
        result.push_back(cat);
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<ShortcutInfo> ShortcutManager::getShortcutsByCategory(const std::string& category) const {
    std::vector<ShortcutInfo> result;
    auto it = categories_.find(category);
    if (it != categories_.end()) {
        for (const auto& action : it->second) {
            auto infoIt = shortcutInfos_.find(action);
            if (infoIt != shortcutInfos_.end()) {
                result.push_back(infoIt->second);
            }
        }
    }
    return result;
}

// ==================== Private Methods ====================

std::optional<std::pair<guint, GdkModifierType>> ShortcutManager::parseAccelerator(const std::string& accel) const {
    guint keyval = 0;
    GdkModifierType state = static_cast<GdkModifierType>(0);
    
    // Split by ">" to handle modifier prefixes like <Ctrl>, <Shift>, etc.
    gchar* accelCopy = g_strdup(accel.c_str());
    gchar** tokens = g_strsplit(accelCopy, ">", 0);
    
    for (int i = 0; tokens[i] != nullptr; i++) {
        gchar* token = tokens[i];
        if (*token == '\0') {
            continue;
        }
        
        // Handle leading '<' that's sometimes present
        if (*token == '<') {
            token++;
        }
        
        // Parse modifier tokens
        if (g_strcmp0(token, "Ctrl") == 0 || g_strcmp0(token, "Control") == 0) {
            state = static_cast<GdkModifierType>(state | GDK_CONTROL_MASK);
        } else if (g_strcmp0(token, "Shift") == 0) {
            state = static_cast<GdkModifierType>(state | GDK_SHIFT_MASK);
        } else if (g_strcmp0(token, "Alt") == 0) {
            state = static_cast<GdkModifierType>(state | GDK_MOD1_MASK);
        } else if (g_strcmp0(token, "Meta") == 0) {
            state = static_cast<GdkModifierType>(state | GDK_META_MASK);
        } else if (g_strcmp0(token, "Primary") == 0) {
            // Primary is typically Ctrl on most platforms
            state = static_cast<GdkModifierType>(state | GDK_CONTROL_MASK);
        } else {
            // Should be the actual key - use gdk to convert name to keyval
            guint kv = gdk_keyval_from_name(token);
            if (kv != 0) {
                keyval = kv;
            }
        }
    }
    
    g_strfreev(tokens);
    g_free(accelCopy);
    
    if (keyval == 0) {
        return std::nullopt;  // No valid key found
    }
    
    return std::make_pair(keyval, state);
}

bool ShortcutManager::hasConflict(const std::string& action, const std::string& accelerator) const {
    for (const auto& [otherAction, config] : shortcuts_) {
        if (otherAction == action) {
            continue;  // Skip self
        }
        if (!config.enabled || config.accelerator.empty()) {
            continue;  // Skip disabled shortcuts
        }
        
        // Check for exact accelerator match
        if (config.accelerator == accelerator) {
            return true;
        }
    }
    return false;
}

bool ShortcutManager::validateAccelerator(const std::string& accel) const {
    // Empty is valid (means disable)
    if (accel.empty()) {
        return true;
    }
    // Check if we can parse it
    return parseAccelerator(accel).has_value();
}

// ==================== Import/Export ====================

std::string ShortcutManager::exportToJson() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"version\": 1,\n";
    json << "  \"shortcuts\": {\n";
    
    // Only export shortcuts that differ from defaults
    bool first = true;
    for (const auto& [action, info] : shortcutInfos_) {
        if (!info.isDefault) {
            if (!first) {
                json << ",\n";
            }
            json << "    \"" << action << "\": \"" << info.currentAccel << "\"";
            first = false;
        }
    }
    
    json << "\n  }\n";
    json << "}\n";
    return json.str();
}

bool ShortcutManager::importFromJson(const std::string& jsonStr) {
    // Simple JSON parser for our specific format:
    // { "shortcuts": { "action": "accel", ... } }
    
    // Find "shortcuts" section
    size_t shortcutsStart = jsonStr.find("\"shortcuts\"");
    if (shortcutsStart == std::string::npos) {
        return false;
    }
    
    // Find the opening { after "shortcuts"
    size_t braceStart = jsonStr.find("{", shortcutsStart);
    size_t braceEnd = jsonStr.find("}", braceStart);
    if (braceStart == std::string::npos || braceEnd == std::string::npos) {
        return false;
    }
    
    std::string shortcutsBlock = jsonStr.substr(braceStart + 1, braceEnd - braceStart - 1);
    
    // Parse "action": "accel" pairs
    size_t pos = 0;
    while (pos < shortcutsBlock.size()) {
        // Find opening quote of action name
        size_t actionStart = shortcutsBlock.find("\"", pos);
        if (actionStart == std::string::npos) break;
        
        // Find closing quote of action name
        size_t actionEnd = shortcutsBlock.find("\"", actionStart + 1);
        if (actionEnd == std::string::npos) break;
        
        std::string action = shortcutsBlock.substr(actionStart + 1, actionEnd - actionStart - 1);
        
        // Find colon
        size_t colonStart = shortcutsBlock.find(":", actionEnd);
        if (colonStart == std::string::npos) break;
        
        // Find opening quote of accelerator value
        size_t accelStart = shortcutsBlock.find("\"", colonStart);
        if (accelStart == std::string::npos) break;
        
        // Find closing quote of accelerator value
        size_t accelEnd = shortcutsBlock.find("\"", accelStart + 1);
        if (accelEnd == std::string::npos) break;
        
        std::string accel = shortcutsBlock.substr(accelStart + 1, accelEnd - accelStart - 1);
        
        // Apply this shortcut
        setShortcut(action, accel);
        
        // Move to next entry
        pos = accelEnd + 1;
    }
    
    return true;
}
