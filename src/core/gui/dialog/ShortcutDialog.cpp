/*
 * Xournal++ Shortcut Settings Dialog
 * 
 * Provides a GUI for viewing and customizing keyboard shortcuts.
 * See ShortcutDialog.h for details.
 */

#include "ShortcutDialog.h"

#include <cstring>
#include <string>
#include <algorithm>

#include <gtk/gtk.h>

#include "control/Control.h"
#include "control/settings/ShortcutManager.h"
#include "control/settings/Settings.h"
#include "gui/Builder.h"
#include "gui/GladeSearchpath.h"
#include "util/i18n.h"

// UI definition file for the shortcut settings dialog
constexpr auto UI_FILE = "shortcutSettings.glade";

// ==================== Local Types ====================
struct DialogInfo {
    GtkWidget* dialog;
    std::string accel;
};

// ==================== Constructor / Destructor ====================

ShortcutDialog::ShortcutDialog(GladeSearchpath* gladeSearchPath, Control* control):
        control(control),
        builder(new Builder(gladeSearchPath, UI_FILE)) {
    // Get the dialog window from the Builder
    this->window = builder->get("shortcutSettings");
}

ShortcutDialog::~ShortcutDialog() {
    gtk_widget_destroy(window);
    delete builder;
}

// ==================== Public Methods ====================

void ShortcutDialog::show(GtkWindow* parent) {
    // Set up modal dialog behavior
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    
    // Initialize the category and shortcut lists
    initCategoryList();
    initShortcutList();
    
    // Connect search entry to filter shortcuts
    GtkSearchEntry* searchEntry = GTK_SEARCH_ENTRY(builder->get("searchEntry"));
    g_signal_connect(searchEntry, "search-changed", G_CALLBACK(+[](GtkSearchEntry* entry, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->searchText = gtk_entry_get_text(GTK_ENTRY(entry));
        dialog->updateShortcutList();
    }), this);
    
    // Connect window close (X button) to quit
    g_signal_connect_swapped(window, "delete-event", G_CALLBACK(+[](GtkWidget* /*w*/) {
        gtk_main_quit();
        return FALSE;
    }), nullptr);
    
    // Show window - this call blocks until gtk_main_quit() is called
    gtk_widget_show_all(window);
    gtk_main();
}

// ==================== Initialization ====================

void ShortcutDialog::initCategoryList() {
    GtkTreeView* categoryView = GTK_TREE_VIEW(builder->get("categoryTreeView"));
    
    // Create a simple list store with one column (category name)
    GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(categoryView, GTK_TREE_MODEL(store));
    
    // Add text renderer for the category column
    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(categoryView, -1, _("Category"), renderer, "text", 0, nullptr);
    
    // Populate categories from ShortcutManager
    auto* sm = control->getShortcutManager();
    auto categories = sm->getCategories();
    
    for (const auto& cat : categories) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, cat.c_str(), -1);
    }
    
    // Select the first category by default
    if (!categories.empty()) {
        GtkTreePath* path = gtk_tree_path_new_from_string("0");
        GtkTreeSelection* selection = gtk_tree_view_get_selection(categoryView);
        gtk_tree_selection_select_path(selection, path);
        selectedCategory = categories[0];
        gtk_tree_path_free(path);
    }
}

void ShortcutDialog::initShortcutList() {
    GtkTreeView* shortcutView = GTK_TREE_VIEW(builder->get("shortcutTreeView"));
    
    // Create list store with 6 columns: action_id, category, display_name, current_accel, default_accel, description
    GtkListStore* store = gtk_list_store_new(6, 
        G_TYPE_STRING,  // 0: action_id (hidden)
        G_TYPE_STRING,  // 1: category (Category)
        G_TYPE_STRING,  // 2: display_name (Action)
        G_TYPE_STRING,  // 3: current_accel (Current)
        G_TYPE_STRING,  // 4: default_accel (Default)
        G_TYPE_STRING); // 5: description (Description)
    gtk_tree_view_set_model(shortcutView, GTK_TREE_MODEL(store));
    
    // Column 1: Category (read-only text)
    GtkCellRenderer* catRenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(shortcutView, -1, _("Category"), catRenderer, "text", 1, nullptr);
    GtkTreeViewColumn* col1 = gtk_tree_view_get_column(shortcutView, 0);
    if (col1) {
        gtk_tree_view_column_set_resizable(col1, TRUE);
        gtk_tree_view_column_set_sizing(col1, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
        gtk_tree_view_column_set_expand(col1, TRUE);
    }
    
    // Column 2: Display name (read-only text)
    GtkCellRenderer* textRenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(shortcutView, -1, _("Action"), textRenderer, "text", 2, nullptr);
    GtkTreeViewColumn* col2 = gtk_tree_view_get_column(shortcutView, 1);
    if (col2) {
        gtk_tree_view_column_set_resizable(col2, TRUE);
        gtk_tree_view_column_set_sizing(col2, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
        gtk_tree_view_column_set_expand(col2, TRUE);
    }
    
    // Column 3: Current accelerator (editable)
    GtkCellRenderer* accelRenderer = gtk_cell_renderer_accel_new();
    g_object_set(accelRenderer, "editable", TRUE, "accel-mode", GTK_CELL_RENDERER_ACCEL_MODE_OTHER, nullptr);
    g_signal_connect(accelRenderer, "accel-edited", G_CALLBACK(+[](GtkCellRendererAccel* r, gchar* path, guint keyval, 
                           GdkModifierType mask, guint /* hw_keycode */, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->onShortcutEdited(path, keyval, mask);
    }), this);
    g_signal_connect(accelRenderer, "accel-cancelled", G_CALLBACK(+[](GtkCellRendererAccel* r, gchar* path, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->onShortcutEditCancelled(path);
    }), this);
    
    gtk_tree_view_insert_column_with_attributes(shortcutView, -1, _("Current"), accelRenderer, "text", 3, nullptr);
    GtkTreeViewColumn* col3 = gtk_tree_view_get_column(shortcutView, 2);
    if (col3) {
        gtk_tree_view_column_set_resizable(col3, TRUE);
        gtk_tree_view_column_set_sizing(col3, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
        gtk_tree_view_column_set_fixed_width(col3, 180);
    }
    
    // Column 4: Default accelerator (read-only text)
    GtkCellRenderer* defaultRenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(shortcutView, -1, _("Default"), defaultRenderer, "text", 4, nullptr);
    GtkTreeViewColumn* col4 = gtk_tree_view_get_column(shortcutView, 3);
    if (col4) {
        gtk_tree_view_column_set_resizable(col4, TRUE);
        gtk_tree_view_column_set_sizing(col4, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
        gtk_tree_view_column_set_fixed_width(col4, 180);
    }
    
    // Column 5: Description (read-only text)
    GtkCellRenderer* descRenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(shortcutView, -1, _("Description"), descRenderer, "text", 5, nullptr);
    GtkTreeViewColumn* col5 = gtk_tree_view_get_column(shortcutView, 4);
    if (col5) {
        gtk_tree_view_column_set_resizable(col5, TRUE);
        gtk_tree_view_column_set_sizing(col5, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
        gtk_tree_view_column_set_expand(col5, TRUE);
    }
    
    // Connect category selection to update shortcut list
    GtkTreeView* categoryView = GTK_TREE_VIEW(builder->get("categoryTreeView"));
    g_signal_connect(categoryView, "cursor_changed", G_CALLBACK(+[](GtkTreeView* tv, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->onCategoryChanged(tv);
    }), this);
    
    // Connect action buttons using lambda wrappers to call member methods
    g_signal_connect(builder->get("btnResetSelected"), "clicked", G_CALLBACK(+[](GtkButton* /*btn*/, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->resetSelected();
    }), this);
    
    g_signal_connect(builder->get("btnResetAll"), "clicked", G_CALLBACK(+[](GtkButton* /*btn*/, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->resetAll();
    }), this);
    
    g_signal_connect(builder->get("btnExport"), "clicked", G_CALLBACK(+[](GtkButton* /*btn*/, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->exportShortcuts();
    }), this);
    
    g_signal_connect(builder->get("btnImport"), "clicked", G_CALLBACK(+[](GtkButton* /*btn*/, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->importShortcuts();
    }), this);
    
    g_signal_connect(builder->get("btnClose"), "clicked", G_CALLBACK(+[](GtkButton* /*btn*/, gpointer data) {
        auto* dialog = static_cast<ShortcutDialog*>(data);
        dialog->saveAndClose();
    }), this);
    
    // Populate the shortcut list for the initially selected category
    updateShortcutList();
}

void ShortcutDialog::updateShortcutList() {
    if (selectedCategory.empty()) {
        return;
    }
    
    GtkTreeView* shortcutView = GTK_TREE_VIEW(builder->get("shortcutTreeView"));
    GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(shortcutView));
    gtk_list_store_clear(store);
    
    // Get shortcuts for the selected category
    auto* sm = control->getShortcutManager();
    auto shortcuts = sm->getShortcutsByCategory(selectedCategory);
    
    // Filter and populate the list store
    int count = 0;
    for (const auto& info : shortcuts) {
        // Apply search filter if search text is not empty
        if (!searchText.empty()) {
            std::string searchLower = searchText;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            
            std::string nameLower = info.displayName;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            
            std::string descLower = info.description;
            std::transform(descLower.begin(), descLower.end(), descLower.begin(), ::tolower);
            
            std::string actionLower = info.action;
            std::transform(actionLower.begin(), actionLower.end(), actionLower.begin(), ::tolower);
            
            std::string currentLower = info.currentAccel;
            std::transform(currentLower.begin(), currentLower.end(), currentLower.begin(), ::tolower);
            
            std::string defaultLower = info.defaultAccel;
            std::transform(defaultLower.begin(), defaultLower.end(), defaultLower.begin(), ::tolower);
            
            bool matches = nameLower.find(searchLower) != std::string::npos ||
                          descLower.find(searchLower) != std::string::npos ||
                          actionLower.find(searchLower) != std::string::npos ||
                          currentLower.find(searchLower) != std::string::npos ||
                          defaultLower.find(searchLower) != std::string::npos;
            if (!matches) {
                continue;
            }
        }
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 
                          0, info.action.c_str(),        // Column 0: action ID (hidden)
                          1, info.category.c_str(),       // Column 1: Category
                          2, info.displayName.c_str(),   // Column 2: Action name
                          3, info.currentAccel.c_str(),  // Column 3: Current accelerator
                          4, info.defaultAccel.c_str(),  // Column 4: Default accelerator
                          5, info.description.c_str(),   // Column 5: Description
                          -1);
        count++;
    }
    
    // Update result count label
    GtkWidget* countLabel = GTK_WIDGET(builder->get("searchResultCount"));
    if (searchText.empty()) {
        gtk_label_set_text(GTK_LABEL(countLabel), _("Showing all shortcuts"));
    } else {
        std::string text = g_strdup_printf(_("Showing %d of %d shortcuts"), count, static_cast<int>(shortcuts.size()));
        gtk_label_set_text(GTK_LABEL(countLabel), text.c_str());
    }
}

// ==================== Event Handlers ====================

void ShortcutDialog::onCategoryChanged(GtkTreeView* treeView) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* category;
        gtk_tree_model_get(model, &iter, 0, &category, -1);
        selectedCategory = category;
        g_free(category);
        updateShortcutList();
    }
}

void ShortcutDialog::onShortcutEdited(const gchar* path, guint keyval, GdkModifierType mask) {
    GtkTreeView* shortcutView = GTK_TREE_VIEW(builder->get("shortcutTreeView"));
    GtkTreeModel* model = gtk_tree_view_get_model(shortcutView);
    GtkTreePath* treePath = gtk_tree_path_new_from_string(path);
    GtkTreeIter iter;
    
    // Get the action name from the selected row
    if (!gtk_tree_model_get_iter(model, &iter, treePath)) {
        gtk_tree_path_free(treePath);
        return;
    }
    
    gchar* action;
    gtk_tree_model_get(model, &iter, 0, &action, -1);
    
    // Convert keyval and mask to accelerator string
    // gtk_accelerator_name returns lowercase for letters, but we want uppercase
    gchar* accelStr = gtk_accelerator_name(keyval, mask);
    if (accelStr) {
        // Uppercase the key part (after the last '>' or the whole string if no modifiers)
        gchar** parts = g_strsplit(accelStr, ">", -1);
        for (int i = 0; parts[i] != nullptr; i++) {
            if (parts[i][0] != '\0') {
                gchar* upper = g_ascii_strup(parts[i], -1);
                g_free(parts[i]);
                parts[i] = upper;
            }
        }
        gchar* joined = g_strjoinv(">", parts);
        g_strfreev(parts);
        std::string accel = joined;
        g_free(accelStr);
        g_free(joined);
        // Apply the shortcut change
        auto* sm = control->getShortcutManager();
        bool success = sm->setShortcut(action, accel);

        if (success) {
            // Update the display
            gtk_list_store_set(GTK_LIST_STORE(model), &iter, 3, accel.c_str(), -1);
        } else {
            // Conflict: keep original value, show "(conflict)" indicator
            gchar* originalAccel = nullptr;
            gtk_tree_model_get(model, &iter, 3, &originalAccel, -1);
            if (originalAccel) {
                std::string str(originalAccel);
                g_free(originalAccel);
                // Strip existing "(conflict)" if present
                size_t pos = str.find(" (conflict)");
                if (pos != std::string::npos) {
                    str = str.substr(0, pos);
                }
                if (str.empty()) str = "(none)";
                std::string display = str + " (conflict)";
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, 3, display.c_str(), -1);
            }
        }
        gtk_tree_path_free(treePath);
        g_free(action);
        return;
    }

    // accelStr was null - cleanup and return
    gtk_tree_path_free(treePath);
    g_free(action);
}

void ShortcutDialog::onShortcutEditCancelled(const gchar* path) {
    GtkTreeView* shortcutView = GTK_TREE_VIEW(builder->get("shortcutTreeView"));
    GtkTreeModel* model = gtk_tree_view_get_model(shortcutView);
    GtkTreePath* treePath = gtk_tree_path_new_from_string(path);
    GtkTreeIter iter;
    
    if (!gtk_tree_model_get_iter(model, &iter, treePath)) {
        gtk_tree_path_free(treePath);
        return;
    }
    gtk_tree_path_free(treePath);
    
    // Clear "(conflict)" indicator if present
    gchar* text = nullptr;
    gtk_tree_model_get(model, &iter, 3, &text, -1);
    if (text) {
        std::string str(text);
        g_free(text);
        size_t pos = str.find(" (conflict)");
        if (pos != std::string::npos) {
            str = str.substr(0, pos);
            if (str.empty()) str = "(none)";
            gtk_list_store_set(GTK_LIST_STORE(model), &iter, 3, str.c_str(), -1);
        }
    }
}

// ==================== Actions ====================

void ShortcutDialog::resetSelected() {
    GtkTreeView* shortcutView = GTK_TREE_VIEW(builder->get("shortcutTreeView"));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(shortcutView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* action;
        gtk_tree_model_get(model, &iter, 0, &action, -1);
        
        auto* sm = control->getShortcutManager();
        sm->resetToDefault(action);
        
        // Update display with the new (default) accelerator
        auto info = sm->getShortcut(action);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 3, info.accelerator.c_str(), -1);
        
        g_free(action);
    }
}

void ShortcutDialog::resetAll() {
    // Ask for confirmation before resetting all
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_QUESTION,
                                              GTK_BUTTONS_YES_NO,
                                              _("Reset all shortcuts to default?"));
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if (response == GTK_RESPONSE_YES) {
        auto* sm = control->getShortcutManager();
        sm->resetAllToDefault();
        updateShortcutList();
    }
}

void ShortcutDialog::exportShortcuts() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Export Shortcuts"),
                                                    GTK_WINDOW(window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT,
                                                    nullptr);
    
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "shortcuts.json");
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT) {
        gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Generate JSON and write to file
        auto* sm = control->getShortcutManager();
        std::string json = sm->exportToJson();
        
        GError* error = nullptr;
        if (!g_file_set_contents(filename, json.c_str(), json.size(), &error)) {
            GtkWidget* errDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                                         GTK_MESSAGE_ERROR,
                                                         GTK_BUTTONS_OK,
                                                         _("Failed to export: %s"), 
                                                         error->message);
            gtk_dialog_run(GTK_DIALOG(errDialog));
            gtk_widget_destroy(errDialog);
            g_error_free(error);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

void ShortcutDialog::importShortcuts() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Import Shortcuts"),
                                                    GTK_WINDOW(window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    nullptr);
    
    // Add filter for JSON files only
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.json");
    gtk_file_filter_set_name(filter, _("JSON files"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT) {
        gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        GError* error = nullptr;
        gsize length;
        gchar* content;
        
        if (g_file_get_contents(filename, &content, &length, &error)) {
            // Parse and apply the imported shortcuts
            auto* sm = control->getShortcutManager();
            bool success = sm->importFromJson(content);
            
            if (success) {
                updateShortcutList();
                
                GtkWidget* msgDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                                            GTK_MESSAGE_INFO,
                                                            GTK_BUTTONS_OK,
                                                            _("Shortcuts imported successfully!"));
                gtk_dialog_run(GTK_DIALOG(msgDialog));
                gtk_widget_destroy(msgDialog);
            } else {
                GtkWidget* errDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            _("Invalid shortcuts file format!"));
                gtk_dialog_run(GTK_DIALOG(errDialog));
                gtk_widget_destroy(errDialog);
            }
            
            g_free(content);
        } else {
            GtkWidget* errDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_OK,
                                                        _("Failed to read file: %s"), 
                                                        error->message);
            gtk_dialog_run(GTK_DIALOG(errDialog));
            gtk_widget_destroy(errDialog);
            g_error_free(error);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

void ShortcutDialog::saveAndClose() {
    auto* sm = control->getShortcutManager();
    sm->saveToSettings();
    gtk_widget_hide(window);
    gtk_main_quit();
}
