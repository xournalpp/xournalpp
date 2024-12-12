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

#include "GladeSearchpath.h"  // for GladeSearchpath
#include "MainWindow.h"
#include "ToolbarDefinitions.h"  // for TOOLBAR_DEFINITIO...
#include "XournalView.h"         // for XournalView
#include "config-dev.h"          // for TOOLBAR_CONFIG
#include "filesystem.h"          // for path, exists

HomeWindow::HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app, MainWindow* win):
        GladeGui(gladeSearchPath, "homepage.glade", "windowhome"), control(control), win(win) {
    gtk_window_set_application(GTK_WINDOW(getWindow()), app);

    // Set window to maximized
    gtk_window_maximize(GTK_WINDOW(getWindow()));

    // Set window position to center
    gtk_window_set_position(GTK_WINDOW(getWindow()), GTK_WIN_POS_CENTER);

    // Connect the button signals
    getWindow();
    initHomeWidget();

    Builder builder(gladeSearchPath, "homepage.glade");

    /* GtkWidget* buttonNewDocument = this->get("newDocument_button");
    GtkWidget* buttonOpenRecentDocument = this->get("openRecentDocument_button");
    if (buttonNewDocument && buttonOpenRecentDocument) {
        g_signal_connect(buttonNewDocument, "clicked", G_CALLBACK(on_buttonNewDocument_clicked), this);
        g_signal_connect(buttonOpenRecentDocument, "clicked", G_CALLBACK(on_buttonOpenRecentDocument_clicked), this);
    } else {
        g_warning("Buttons not found in Glade file.");
    } */

    // Getting GtkGrid from homepage.glade
    this->recentDocumentsGrid = this->get("recentDocumentsGrid");

    gtk_widget_get_allocation(this->window, &allocation);
    int button_width = static_cast<gint>(allocation.width * 0.25);
    int button_height = 400;

    GtkWidget* buttonNewDocument = gtk_button_new_with_label("New Document");
    gtk_widget_set_size_request(buttonNewDocument, button_width, button_height);
    g_signal_connect(buttonNewDocument, "clicked", G_CALLBACK(on_buttonNewDocument_clicked), this);

    gtk_grid_attach(GTK_GRID(this->recentDocumentsGrid), buttonNewDocument, 0, 0, 1, 1);
    
    createRecentDocumentButtons(button_width, button_height);

    std::cout << "HomeWindow created\n";
}

std::vector<std::string> HomeWindow::getRecentDocuments() {
    // Placeholder for actual logic to get recent documents
    // This should be replaced with actual logic to fetch recent documents
    return {"/path/to/recent/document1.txt", "/path/to/recent/document2.txt", "/path/to/recent/document3.txt",
            "/path/to/recent/document4.txt", "/path/to/recent/document5.txt", "/path/to/recent/document6.txt",
            "/path/to/recent/document7.txt", "/path/to/recent/document8.txt", "/path/to/recent/document9.txt"};
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

void HomeWindow::createRecentDocumentButtons(int button_width, int button_height) {
    std::vector<std::string> recentDocuments = getRecentDocuments();
    int row = 0, col = 1;

    for (const auto& doc : recentDocuments) {
        GtkWidget* button = gtk_button_new_with_label(std::filesystem::path(doc).filename().c_str());
        gtk_widget_set_size_request(button, button_width, button_height);
        g_signal_connect(button, "clicked", G_CALLBACK(on_recent_document_button_clicked), (gpointer)doc.c_str());

        gtk_grid_attach(GTK_GRID(this->recentDocumentsGrid), button, col, row, 1, 1);
        col++;
        if (col == 4) {
            col = 0;
            row++;
        }
    }

    gtk_widget_show_all(this->recentDocumentsGrid);
}

void HomeWindow::on_recent_document_button_clicked(GtkButton* button, gpointer user_data) {
    const char* doc_path = static_cast<const char*>(user_data);
    std::cout << "Opening document: " << doc_path << std::endl;
    // Logic to open the document
}