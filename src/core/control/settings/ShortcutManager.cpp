#include "ShortcutManager.h"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include <glib.h>
#include <gtk/gtk.h>

#include "control/Control.h"
#include "control/actions/ActionDatabase.h"
#include "control/actions/ActionProperties.h"
#include "control/settings/Settings.h"
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
        return "File";
    
    // Edit operations
    if (action == "undo" || action == "redo" || action == "cut" || action == "copy" || action == "paste" ||
        action == "delete" || action == "select-all" || action == "search")
        return "Edit";
    
    // View operations
    if (action == "fullscreen" || action == "presentation-mode" || action == "show-sidebar" ||
        action == "show-toolbar" || action == "show-menubar" || action == "paired-pages-mode" ||
        action == "paired-pages-offset" || action == "rotation-snapping" || action == "grid-snapping" ||
        action == "toggle-touch-drawing" || action == "position-highlighting" ||
        action == "set-layout-vertical" || action == "set-layout-right-to-left" ||
        action == "set-layout-bottom-to-top" || action == "set-columns-or-rows")
        return "View";
    
    // Page operations
    if (action == "goto-first" || action == "goto-previous" || action == "goto-next" ||
        action == "goto-last" || action == "goto-page" || action == "goto-next-annotated-page" ||
        action == "goto-previous-annotated-page" || action == "new-page-before" ||
        action == "new-page-after" || action == "new-page-at-end" || action == "duplicate-page" ||
        action == "delete-page" || action == "move-page-towards-beginning" ||
        action == "move-page-towards-end" || action == "paper-format" ||
        action == "paper-background-color" || action == "configure-page-template")
        return "Page";
    
    // Layer operations
    if (action == "layer-new" || action == "layer-delete" || action == "layer-show-all" ||
        action == "layer-hide-all" || action == "layer-merge-down" || action == "layer-copy" ||
        action == "layer-move-up" || action == "layer-move-down" || action == "layer-goto-next" ||
        action == "layer-goto-previous" || action == "layer-goto-top" || action == "layer-active" ||
        action == "layer-rename")
        return "Layer";
    
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
        return "Tools";
    
    // Zoom operations
    if (action == "zoom-in" || action == "zoom-out" || action == "zoom-100" || action == "zoom-fit" || action == "zoom")
        return "Zoom";
    
    // Navigation
    if (action == "navigate-back" || action == "navigate-forward")
        return "Navigation";
    
    // Audio
    if (action == "audio-record" || action == "audio-playback" || action == "audio-stop" ||
        action == "audio-seek-forwards" || action == "audio-seek-backwards")
        return "Audio";
    
    // Special
    if (action == "preferences" || action == "customize-shortcuts" || action == "customize-toolbar" ||
        action == "manage-toolbar" || action == "plugin-manager" || action == "help" || action == "about" ||
        action == "font" || action == "select-font" || action == "select-color" || action == "tex" ||
        action == "arrange-selection-order" || action == "move-selection-layer-up" ||
        action == "move-selection-layer-down")
        return "Special";
    
    return "General";
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
    // Check locale and return appropriate translation
    const char* locale = setlocale(LC_MESSAGES, nullptr);
    bool isChinese = locale && (strstr(locale, "zh") != nullptr);
    
    // Helper macro for bilingual descriptions
    #define DESC(en, zh) isChinese ? (zh) : (en)
    
    if (action == "new-file") return DESC("Create a new document", "新建文档");
    if (action == "open") return DESC("Open an existing document", "打开文档");
    if (action == "annotate-pdf") return DESC("Annotate an existing PDF", "为 PDF 添加注释");
    if (action == "save") return DESC("Save the current document", "保存当前文档");
    if (action == "save-as") return DESC("Save with a new name", "另存为");
    if (action == "export-as-pdf") return DESC("Export document as PDF", "导出为 PDF");
    if (action == "export-as") return DESC("Export in various formats", "导出为其他格式");
    if (action == "print") return DESC("Print the document", "打印文档");
    if (action == "quit") return DESC("Exit the application", "退出应用");
    if (action == "undo") return DESC("Undo the last action", "撤销");
    if (action == "redo") return DESC("Redo the last undone action", "重做");
    if (action == "cut") return DESC("Cut selection to clipboard", "剪切");
    if (action == "copy") return DESC("Copy selection to clipboard", "复制");
    if (action == "paste") return DESC("Paste from clipboard", "粘贴");
    if (action == "search") return DESC("Search in document", "搜索文档");
    if (action == "select-all") return DESC("Select all content", "全选");
    if (action == "delete") return DESC("Delete selection", "删除选中内容");
    if (action == "zoom-in") return DESC("Increase zoom level", "放大");
    if (action == "zoom-out") return DESC("Decrease zoom level", "缩小");
    if (action == "zoom-100") return DESC("Reset zoom to 100%", "重置缩放至 100%");
    if (action == "zoom-fit") return DESC("Fit content to window", "适合窗口");
    if (action == "fullscreen") return DESC("Toggle fullscreen mode", "全屏模式");
    if (action == "show-sidebar") return DESC("Toggle sidebar visibility", "显示/隐藏侧边栏");
    if (action == "show-toolbar") return DESC("Toggle toolbar visibility", "显示/隐藏工具栏");
    if (action == "show-menubar") return DESC("Toggle menubar visibility", "显示/隐藏菜单栏");
    if (action == "preferences") return DESC("Open settings dialog", "打开设置");
    if (action == "customize-shortcuts") return DESC("Customize keyboard shortcuts", "自定义快捷键");
    if (action == "goto-first") return DESC("Go to first page", "转到第一页");
    if (action == "goto-previous") return DESC("Go to previous page", "转到上一页");
    if (action == "goto-next") return DESC("Go to next page", "转到下一页");
    if (action == "goto-last") return DESC("Go to last page", "转到最后一页");
    if (action == "new-page-before") return DESC("Insert page before current", "在当前页前插入");
    if (action == "new-page-after") return DESC("Insert page after current", "在当前页后插入");
    if (action == "new-page-at-end") return DESC("Append new page at end", "在末尾添加新页");
    if (action == "duplicate-page") return DESC("Duplicate current page", "复制当前页");
    if (action == "delete-page") return DESC("Delete current page", "删除当前页");
    if (action == "select-tool") return DESC("Select tool", "选择工具");
    if (action == "select-default-tool") return DESC("Select default tool", "选择默认工具");
    if (action == "tool-pen-size") return DESC("Change pen thickness", "画笔粗细");
    if (action == "tool-pen-line-style") return DESC("Change line style", "线条样式");
    if (action == "tool-pen-fill") return DESC("Toggle pen fill", "画笔填充");
    if (action == "tool-pen-fill-opacity") return DESC("Pen fill opacity", "画笔填充透明度");
    if (action == "tool-eraser-size") return DESC("Change eraser size", "橡皮擦大小");
    if (action == "tool-eraser-type") return DESC("Change eraser type", "橡皮擦类型");
    if (action == "tool-highlighter-size") return DESC("Change highlighter size", "荧光笔大小");
    if (action == "tool-highlighter-fill") return DESC("Toggle highlighter fill", "荧光笔填充");
    if (action == "tool-highlighter-fill-opacity") return DESC("Highlighter opacity", "荧光笔透明度");
    if (action == "tool-draw-shape-recognizer") return DESC("Draw with shape recognition", "形状识别绘图");
    if (action == "tool-draw-rectangle") return DESC("Draw rectangle", "绘制矩形");
    if (action == "tool-draw-ellipse") return DESC("Draw ellipse", "绘制椭圆");
    if (action == "tool-draw-arrow") return DESC("Draw arrow", "绘制箭头");
    if (action == "tool-draw-double-arrow") return DESC("Draw double arrow", "绘制双箭头");
    if (action == "tool-draw-line") return DESC("Draw straight line", "绘制直线");
    if (action == "tool-draw-spline") return DESC("Draw spline", "绘制样条曲线");
    if (action == "tool-draw-coordinate-system") return DESC("Draw coordinate system", "绘制坐标系");
    if (action == "tool-color") return DESC("Change drawing color", "更改绘图颜色");
    if (action == "tool-size") return DESC("Change tool size", "工具大小");
    if (action == "tool-fill") return DESC("Toggle fill", "切换填充");
    if (action == "tool-fill-opacity") return DESC("Fill opacity", "填充透明度");
    if (action == "layer-show-all") return DESC("Show all layers", "显示所有图层");
    if (action == "layer-hide-all") return DESC("Hide all layers", "隐藏所有图层");
    if (action == "layer-new") return DESC("Create new layer", "新建图层");
    if (action == "layer-delete") return DESC("Delete current layer", "删除当前图层");
    if (action == "layer-merge-down") return DESC("Merge with layer below", "与下方图层合并");
    if (action == "layer-copy") return DESC("Copy layer", "复制图层");
    if (action == "layer-move-up") return DESC("Move layer up", "图层层级上移");
    if (action == "layer-move-down") return DESC("Move layer down", "图层层级下移");
    if (action == "layer-goto-next") return DESC("Go to next layer", "转到下一个图层");
    if (action == "layer-goto-previous") return DESC("Go to previous layer", "转到上一个图层");
    if (action == "layer-goto-top") return DESC("Go to top layer", "转到顶层");
    if (action == "layer-active") return DESC("Toggle layer visibility", "切换图层可见性");
    if (action == "layer-rename") return DESC("Rename layer", "重命名图层");
    if (action == "presentation-mode") return DESC("Toggle presentation mode", "演示模式");
    if (action == "paired-pages-mode") return DESC("Toggle paired pages", "双页模式");
    if (action == "paired-pages-offset") return DESC("Toggle paired pages offset", "双页偏移");
    if (action == "rotation-snapping") return DESC("Toggle rotation snapping", "旋转吸附");
    if (action == "grid-snapping") return DESC("Toggle grid snapping", "网格吸附");
    if (action == "toggle-touch-drawing") return DESC("Toggle touch drawing", "触屏绘图");
    if (action == "position-highlighting") return DESC("Toggle position highlighting", "位置高亮");
    if (action == "set-layout-vertical") return DESC("Set vertical layout", "垂直布局");
    if (action == "set-layout-right-to-left") return DESC("Set right-to-left layout", "从右到左布局");
    if (action == "set-layout-bottom-to-top") return DESC("Set bottom-to-top layout", "从下到上布局");
    if (action == "set-columns-or-rows") return DESC("Set columns or rows", "设置列或行");
    if (action == "paper-background-color") return DESC("Paper background color", "纸张背景色");
    if (action == "paper-format") return DESC("Paper format", "纸张格式");
    if (action == "configure-page-template") return DESC("Configure page template", "配置页面模板");
    if (action == "font") return DESC("Select font", "选择字体");
    if (action == "select-font") return DESC("Select font", "选择字体");
    if (action == "tex") return DESC("Insert LaTeX", "插入 LaTeX");
    if (action == "compass") return DESC("Toggle compass tool", "圆规工具");
    if (action == "setsquare") return DESC("Toggle set square tool", "三角尺工具");
    if (action == "plugin-manager") return DESC("Plugin manager", "插件管理器");
    if (action == "help") return DESC("Show help", "显示帮助");
    if (action == "about") return DESC("Show about dialog", "关于");
    if (action == "audio-record") return DESC("Record audio", "录音");
    if (action == "audio-playback") return DESC("Play/pause audio", "播放/暂停音频");
    if (action == "audio-stop") return DESC("Stop audio", "停止音频");
    if (action == "audio-seek-forwards") return DESC("Seek forwards", "快进");
    if (action == "audio-seek-backwards") return DESC("Seek backwards", "快退");
    if (action == "move-page-towards-beginning") return DESC("Move page towards beginning", "向前移动页面");
    if (action == "move-page-towards-end") return DESC("Move page towards end", "向后移动页面");
    if (action == "append-new-pdf-pages") return DESC("Append new PDF pages", "追加新 PDF 页面");
    if (action == "navigate-back") return DESC("Navigate back", "后退");
    if (action == "navigate-forward") return DESC("Navigate forward", "前进");
    if (action == "goto-page") return DESC("Go to page dialog", "转到页面对话框");
    if (action == "goto-next-annotated-page") return DESC("Go to next annotated page", "转到下一个有注释的页面");
    if (action == "goto-previous-annotated-page") return DESC("Go to previous annotated page", "转到上一个有注释的页面");
    
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
    std::string actionName = ActionNamespaceHelper<a>::NAMESPACE;
    actionName += Action_toString(a);
    
    // Use human-readable display name and determine category
    std::string displayName = actionToDisplayName(actionName);
    std::string category = getActionCategory(actionName);
    
    // Register with first accelerator if available, otherwise empty (still shows in dialog)
    std::string defaultAccel = (accels && accels[0] != nullptr) ? accels[0] : "";
    sm->registerShortcut(actionName, category, displayName, defaultAccel);
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
