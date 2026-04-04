/*
 * Xournal++
 *
 * Shortcut Manager - Manages customizable keyboard shortcuts for the application.
 * Provides registration, querying, modification, and persistence of shortcuts.
 *
 * @author Xournal++ Team
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <functional>

#include <gdk/gdk.h>

class Control;

/**
 * Represents the configuration for a single keyboard shortcut.
 * Stores the GTK accelerator string and enabled state.
 */
struct ShortcutConfig {
    std::string accelerator;  // GTK accelerator string, e.g. "<Ctrl>plus" or "<Primary>plus"
    bool enabled = true;      // Whether this shortcut is currently active
    
    ShortcutConfig() = default;
    ShortcutConfig(const std::string& accel, bool en = true) 
        : accelerator(accel), enabled(en) {}
};

/**
 * Detailed information about a shortcut, including its display name,
 * default value, current value, and whether it matches the default.
 */
struct ShortcutInfo {
    std::string action;       // Full action name, e.g. "win.quit" or "app.open"
    std::string category;     // Category for grouping, e.g. "File", "Edit", "View"
    std::string displayName;  // Human-readable name for UI display
    std::string description;   // What this action does (translatable)
    std::string defaultAccel; // Default accelerator string from ActionProperties
    std::string currentAccel; // Current accelerator (may be customized)
    bool enabled;             // Whether the shortcut is currently enabled
    bool isDefault;           // Whether current accelerator matches the default
};

/**
 * ShortcutManager - Central manager for all keyboard shortcuts.
 * 
 * This class provides a unified interface for:
 * - Registering shortcuts (from ActionProperties or manually)
 * - Querying shortcut configuration
 * - Modifying shortcuts (with conflict detection)
 * - Persisting customizations to Settings
 * - Importing/exporting shortcut configurations
 * 
 * Shortcuts are stored in two maps:
 * - shortcuts_: Maps action name to ShortcutConfig (runtime state)
 * - shortcutInfos_: Maps action name to ShortcutInfo (metadata and defaults)
 */
class ShortcutManager {
public:
    explicit ShortcutManager(Control* control);
    ~ShortcutManager() = default;

    // Non-copyable, non-movable
    ShortcutManager(const ShortcutManager&) = delete;
    ShortcutManager& operator=(const ShortcutManager&) = delete;

    // ==================== Registration ====================
    
    /**
     * Register a single shortcut with its metadata.
     * 
     * @param action Full action name (e.g. "win.QUIT", "app.OPEN")
     * @param category Category for grouping in UI
     * @param displayName Human-readable name for display
     * @param defaultAccel Default accelerator string
     */
    void registerShortcut(const std::string& action, 
                        const std::string& category,
                        const std::string& displayName,
                        const std::string& defaultAccel);
    
    /**
     * Register multiple shortcuts at once.
     * 
     * @param shortcuts Vector of (action, category, displayName, defaultAccel) tuples
     */
    void registerShortcuts(const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& shortcuts);

    /**
     * Register all default shortcuts from ActionProperties template specializations.
     * This iterates through all Action enum values and registers shortcuts
     * for those that have accelerators defined in ActionProperties.
     */
    void registerDefaultShortcuts();

    // ==================== Query ====================
    
    /**
     * Get the current configuration for a specific action.
     * 
     * @param action Full action name
     * @return ShortcutConfig with current accelerator and enabled state
     */
    ShortcutConfig getShortcut(const std::string& action) const;
    
    /**
     * Get all registered shortcuts with their full information.
     * 
     * @return Vector of ShortcutInfo for all registered shortcuts
     */
    std::vector<ShortcutInfo> getAllShortcuts() const;
    
    /**
     * Get list of all shortcut categories.
     * 
     * @return Vector of unique category names
     */
    std::vector<std::string> getCategories() const;
    
    /**
     * Get all shortcuts within a specific category.
     * 
     * @param category Category name to filter by
     * @return Vector of ShortcutInfo for shortcuts in that category
     */
    std::vector<ShortcutInfo> getShortcutsByCategory(const std::string& category) const;

    // ==================== Modification ====================
    
    /**
     * Set a custom accelerator for an action.
     * Validates the accelerator format and checks for conflicts.
     * Updates both internal state and GTK accelerator map.
     * 
     * @param action Full action name
     * @param accelerator New accelerator string (empty to disable)
     * @return true if successful, false if invalid or conflicting
     */
    bool setShortcut(const std::string& action, const std::string& accelerator);
    
    /**
     * Reset a specific shortcut to its default value.
     * Updates both internal state and GTK accelerator map.
     * 
     * @param action Full action name to reset
     */
    void resetToDefault(const std::string& action);
    
    /**
     * Reset all shortcuts to their default values.
     * Updates both internal state and GTK accelerator map.
     */
    void resetAllToDefault();

    // ==================== Triggering ====================
    
    /**
     * Check if a key event matches any registered shortcut.
     * Used for manual shortcut handling when GTK action system is not used.
     * 
     * @param keyval GDK key value
     * @param state Modifier key state
     * @return true if the key event triggers a shortcut
     */
    bool trigger(guint keyval, GdkModifierType state);

    // ==================== Persistence ====================
    
    /**
     * Load custom shortcuts from Settings.
     * Applied after registerDefaultShortcuts() to override defaults.
     */
    void loadFromSettings();
    
    /**
     * Save custom shortcuts to Settings.
     * Only saves shortcuts that differ from their default values.
     */
    void saveToSettings();

    // ==================== Import/Export ====================
    
    /**
     * Export all shortcuts to JSON format.
     * Includes both default and custom values.
     * 
     * @return JSON string representation of all shortcuts
     */
    std::string exportToJson() const;
    
    /**
     * Import shortcuts from JSON format.
     * Validates format and applies shortcuts that differ from defaults.
     * 
     * @param json JSON string to import
     * @return true if import succeeded, false on format error
     */
    bool importFromJson(const std::string& json);

private:
    Control* control;
    
    // Maps action name to current runtime configuration
    std::unordered_map<std::string, ShortcutConfig> shortcuts_;
    
    // Maps action name to metadata (category, display name, defaults)
    std::unordered_map<std::string, ShortcutInfo> shortcutInfos_;
    
    // Maps category name to list of action names in that category
    std::unordered_map<std::string, std::vector<std::string>> categories_;
    
    /**
     * Parse a GTK accelerator string into keyval and modifier state.
     * 
     * @param accel Accelerator string to parse (e.g. "<Ctrl>plus")
     * @return Optional pair of (keyval, modifiers), or nullopt if invalid
     */
    std::optional<std::pair<guint, GdkModifierType>> parseAccelerator(const std::string& accel) const;
    
    /**
     * Check if an accelerator conflicts with any other registered shortcut.
     * 
     * @param action Action to check (excluded from conflict check)
     * @param accelerator Accelerator string to validate
     * @return true if there is a conflict
     */
    bool hasConflict(const std::string& action, const std::string& accelerator) const;
    
    /**
     * Validate that an accelerator string is properly formatted.
     * 
     * @param accel Accelerator string to validate
     * @return true if valid
     */
    bool validateAccelerator(const std::string& accel) const;
};
