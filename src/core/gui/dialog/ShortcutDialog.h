/*
 * Xournal++ Shortcut Settings Dialog
 * 
 * Provides a GUI for viewing and customizing keyboard shortcuts.
 * Allows users to:
 * - Browse shortcuts by category
 * - Edit individual shortcuts by clicking on them
 * - Reset shortcuts to their default values
 * - Export/import shortcut configurations as JSON
 */

#pragma once

#include <string>

#include <gtk/gtk.h>

class Builder;
class Control;
class GladeSearchpath;

/**
 * Dialog for viewing and customizing keyboard shortcuts.
 * 
 * The dialog displays shortcuts organized by category in a two-pane view:
 * - Left pane: Category list
 * - Right pane: Shortcuts in selected category
 * 
 * Users can click on a shortcut's accelerator to edit it, or use
 * the reset buttons to restore defaults.
 */
class ShortcutDialog {
public:
    /**
     * Construct the shortcut settings dialog.
     * 
     * @param gladeSearchPath Path to search for UI Glade files
     * @param control Main control instance for accessing ShortcutManager
     */
    explicit ShortcutDialog(GladeSearchpath* gladeSearchPath, Control* control);
    ~ShortcutDialog();
    
    /**
     * Show the dialog as a modal dialog.
     * 
     * @param parent Parent window to center on and use as transient window
     */
    void show(GtkWindow* parent);

private:
    Control* control;
    Builder* builder = nullptr;  // Owned builder, initialized in ctor
    GtkWidget* window = nullptr;
    std::string selectedCategory;  // Currently selected category in the UI
    std::string searchText;  // Current search filter text
    
    // ==================== Initialization ====================
    
    /**
     * Populate the category list in the left pane.
     * Gets all categories from ShortcutManager and adds them to the tree model.
     */
    void initCategoryList();
    
    /**
     * Populate the shortcut list in the right pane.
     * Shows shortcuts for the currently selected category.
     */
    void initShortcutList();
    
    /**
     * Refresh the shortcut list display.
     * Called after changes to reflect current state.
     */
    void updateShortcutList();
    
    // ==================== Event Handlers ====================
    
    /**
     * Handle category selection change.
     * Updates the shortcut list to show shortcuts for the new category.
     */
    void onCategoryChanged(GtkTreeView* treeView);
    
    /**
     * Handle shortcut accelerator edited by user.
     * Validates the new accelerator and applies it via ShortcutManager.
     * 
     * @param path Tree path of the edited row
     * @param keyval GDK keyval of the new shortcut
     * @param mask Modifier keys (Ctrl, Shift, Alt, etc.)
     */
    void onShortcutEdited(const gchar* path, guint keyval, GdkModifierType mask);
    
    // ==================== Actions ====================
    
    /**
     * Reset the currently selected shortcut to its default value.
     */
    void resetSelected();
    
    /**
     * Reset all shortcuts in the current category to their default values.
     */
    void resetAll();
    
    /**
     * Export shortcuts to a JSON file via file chooser dialog.
     */
    void exportShortcuts();
    
    /**
     * Import shortcuts from a JSON file via file chooser dialog.
     */
    void importShortcuts();
    
    /**
     * Save shortcuts to settings and close the dialog.
     * Called when user clicks the Close button.
     */
    void saveAndClose();
};
