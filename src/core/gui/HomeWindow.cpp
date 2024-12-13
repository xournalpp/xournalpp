/*
 * HomeWindow.cpp
 *
 * This file contains the implementation of the HomeWindow class, which is responsible for managing the home window of
 * the application. The HomeWindow class provides functionality to initialize and display the home window, handle user
 * interactions, and manage new and recent documents.
 */

#include "HomeWindow.h"

#include <iostream>
#include <regex>

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_new_fr...
#include <gdk/gdk.h>                // for gdk_screen_get_de...
#include <gio/gio.h>                // for g_cancellable_is_...
#include <gtk/gtkcssprovider.h>     // for gtk_css_provider_...

#include "control/AudioController.h"         // for AudioController
#include "control/Control.h"                 // for Control
#include "control/DeviceListHelper.h"        // for getSourceMapping
#include "control/ScrollHandler.h"           // for ScrollHandler
#include "control/XournalMain.h"             // for XournalMain
#include "control/actions/ActionDatabase.h"  // for ActionDatabase
#include "control/jobs/XournalScheduler.h"   // for XournalScheduler
#include "control/layer/LayerController.h"   // for LayerController
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for SCROLLBAR_HIDE_HO...
#include "control/zoom/ZoomControl.h"        // for ZoomControl
#include "gui/Builder.h"
#include "gui/FloatingToolbox.h"                        // for FloatingToolbox
#include "gui/GladeGui.h"                               // for GladeGui
#include "gui/PdfFloatingToolbox.h"                     // for PdfFloatingToolbox
#include "gui/SearchBar.h"                              // for SearchBar
#include "gui/inputdevices/InputEvents.h"               // for INPUT_DEVICE_TOUC...
#include "gui/menus/menubar/Menubar.h"                  // for Menubar
#include "gui/menus/menubar/RecentDocumentsSubmenu.h"   //for gettingrecentdocuments
#include "gui/menus/menubar/ToolbarSelectionSubmenu.h"  // for ToolbarSelectionSubmenu
#include "gui/scroll/ScrollHandling.h"                  // for ScrollHandling
#include "gui/sidebar/Sidebar.h"                        // for Sidebar
#include "gui/toolbarMenubar/ToolMenuHandler.h"         // for ToolMenuHandler
#include "gui/toolbarMenubar/model/ToolbarData.h"       // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarModel.h"      // for ToolbarModel
#include "gui/widgets/SpinPageAdapter.h"                // for SpinPageAdapter
#include "gui/widgets/XournalWidget.h"                  // for gtk_xournal_get_l...
#include "util/GListView.h"                             // for GListView, GListV...
#include "util/PathUtil.h"                              // for getConfigFile
#include "util/Util.h"                                  // for execInUiThread, npos
#include "util/XojMsgBox.h"                             // for XojMsgBox
#include "util/glib_casts.h"                            // for wrap_for_once_v
#include "util/gtk4_helper.h"                           // for gtk_widget_get_width
#include "util/i18n.h"                                  // for FS, _F
#include "util/raii/CStringWrapper.h"                   // for OwnedCString

#include "GladeSearchpath.h"     // for GladeSearchpath
#include "MainWindow.h"          // to get the MainWindow
#include "ToolbarDefinitions.h"  // for TOOLBAR_DEFINITIO...
#include "XournalView.h"         // for XournalView
#include "config-dev.h"          // for TOOLBAR_CONFIG
#include "filesystem.h"          // for path, exists

std::unique_ptr<RecentDocumentsSubmenu> recent;

HomeWindow::HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app, MainWindow* win):
        GladeGui(gladeSearchPath, "homepage.glade", "windowhome"), control(control), win(win) {

    getWindow();
    initHomeWidget();

    Builder builder(gladeSearchPath, "homepage.glade");

    gtk_window_set_application(GTK_WINDOW(getWindow()), app);

    // Set window to maximized
    gtk_window_maximize(GTK_WINDOW(getWindow()));

    // Set window position to center
    gtk_window_set_position(GTK_WINDOW(getWindow()), GTK_WIN_POS_CENTER);

    // Set the label text with markup
    GtkLabel* mainTitle = GTK_LABEL(this->get("main_title"));
    GtkLabel* searchTitle = GTK_LABEL(this->get("search_label"));
    gtk_label_set_markup(mainTitle, "<b><span size='xx-large'>Xournal++ Home Page</span></b>");
    gtk_label_set_markup(searchTitle, "<b><span size='xx-large'>Search: </span></b>");

    // Connect the search entry to the searchDocumentEntry_activate callback
    GtkWidget* searchDocumentEntry = this->get("searchDocument_entry");
    if (searchDocumentEntry) {
        g_signal_connect(searchDocumentEntry, "activate", G_CALLBACK(on_searchDocumentEntry_activate), this);
    } else {
        g_warning("Buttons not found in Glade file.");
    }

    // Set the width of the search entry to 90% of the window width
    gint window_width;
    gtk_window_get_size(GTK_WINDOW(getWindow()), &window_width, nullptr);
    gtk_widget_set_size_request(searchDocumentEntry, static_cast<gint>(window_width * 0.9), -1);

    // Getting GtkGrid from homepage.glade
    this->recentDocumentsGrid = this->get("recentDocumentsGrid");

    // Setting button configs in the grid
    gtk_widget_get_allocation(this->window, &allocation);
    int button_width = static_cast<gint>(allocation.width * 0.25);
    int button_height = 450;

    // Including New Document button in the grid and connect the signal handler
    GtkWidget* buttonNewDocument = gtk_button_new_with_label("New Document");
    gtk_widget_set_size_request(buttonNewDocument, button_width, button_height);
    g_signal_connect(buttonNewDocument, "clicked", G_CALLBACK(on_buttonNewDocument_clicked), this);

    gtk_grid_attach(GTK_GRID(this->recentDocumentsGrid), buttonNewDocument, 0, 0, 1, 1);

    // Calling for the recent documents buttons
    createRecentDocumentButtons(button_width, button_height);

    std::cout << "HomeWindow created\n";
}

std::vector<std::string> HomeWindow::getRecentDocuments() {
    // Logic to get recent documents
    std::vector<std::string> recentDocuments;
    Control* ctrl = this->getControl();
    if (!ctrl) {
        std::cerr << "Control is null\n";
        return recentDocuments;  // Return an empty vector
    }

    auto recent = std::make_unique<RecentDocumentsSubmenu>(ctrl, GTK_APPLICATION_WINDOW(this->getWindow()));
    recent->updateXoppFile();

    for (const auto& path: recent->xoppFiles) {
        recentDocuments.push_back(path.string());
    }

    return recentDocuments;
}

HomeWindow::~HomeWindow() = default;

void HomeWindow::initHomeWidget() {}

void HomeWindow::show(GtkWindow* parent) { gtk_widget_show(this->window); }

void HomeWindow::on_buttonNewDocument_clicked(GtkButton* button, gpointer user_data) {
    // Se espera que se cierre/destruya la ventana de homeWindows quedando la MainWindow.
    HomeWindow* self = static_cast<HomeWindow*>(user_data);
    gtk_widget_destroy(GTK_WIDGET(self->getWindow()));
    gtk_widget_show(GTK_WIDGET(self->win->getWindow()));

    std::cout << "New Document Button clicked!\n";
}

auto HomeWindow::getControl() const -> Control* { return control; }

void HomeWindow::createRecentDocumentButtons(int button_width, int button_height, const gchar* search_text) {
    std::vector<std::string> recentDocuments = getRecentDocuments();
    int row = 0, col = 1;
    
    // If search search_text is not empty, filter the recent documents
    if (g_strcmp0(search_text, "") != 0) {
        std::cout << "Search text: " << search_text << std::endl;
        std::vector<std::string> filteredDocuments;
        std::copy_if(recentDocuments.begin(), recentDocuments.end(), std::back_inserter(filteredDocuments),
                    [&search_text](const std::string& document) {
                        return document.find(search_text) != std::string::npos;
                    });

        recentDocuments = std::move(filteredDocuments);
    }

    for (const auto& doc: recentDocuments) {
        GtkWidget* button = gtk_button_new_with_label(std::filesystem::path(doc).filename().c_str());
        gtk_widget_set_size_request(button, button_width, button_height);
        g_object_set_data(G_OBJECT(button), "home_window", this);
        g_signal_connect(button, "clicked", G_CALLBACK(on_recentDocument_button_clicked), g_strdup(doc.c_str()));

        gtk_grid_attach(GTK_GRID(this->recentDocumentsGrid), button, col, row, 1, 1);
        col++;
        if (col == 4) {
            col = 0;
            row++;
        }
    }

    gtk_widget_show_all(this->recentDocumentsGrid);
}

void HomeWindow::on_recentDocument_button_clicked(GtkButton* button, gpointer user_data) {
    HomeWindow* self = static_cast<HomeWindow*>(g_object_get_data(G_OBJECT(button), "home_window"));
    std::string doc_path = static_cast<const char*>(user_data);
    std::cout << "Opening document: " << doc_path << std::endl;
    // Logic to open the document
    Control* ctrl = self->getControl();
    if (!ctrl) {
        std::cerr << "Control is null\n";
        return;
    }
    std::cout << "Attempting to open file: " << doc_path << '\n';
    ctrl->openFile(doc_path);
    std::cout << "Opened file: " << doc_path << '\n';
    gtk_widget_destroy(GTK_WIDGET(self->getWindow()));
    gtk_widget_show(GTK_WIDGET(self->win->getWindow()));

    g_free(user_data);
}

void HomeWindow::on_searchDocumentEntry_activate(GtkEntry* entry, gpointer user_data) {
    // Search logic for opening files
    const gchar* text = gtk_entry_get_text(entry);
    HomeWindow* self = static_cast<HomeWindow*>(user_data);
    std::cout << "Search Document: " << text << std::endl;
    
    // Delating buttons to clear the grid
    GList* children = gtk_container_get_children(GTK_CONTAINER(self->recentDocumentsGrid));
    for (GList* iter = children; iter != nullptr; iter = iter->next) {
        GtkWidget* button = GTK_WIDGET(iter->data);
        const gchar* button_label = gtk_button_get_label(GTK_BUTTON(button));
        
        if (g_strcmp0(button_label, "New Document") == 0) {
            continue;
        }

        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    // Settings sizes for new buttons, according to search
    gtk_widget_get_allocation(self->window, &self->allocation);
    int button_width = static_cast<gint>(self->allocation.width * 0.25);
    int button_height = 450;

    self->createRecentDocumentButtons(button_width, button_height, text);
}