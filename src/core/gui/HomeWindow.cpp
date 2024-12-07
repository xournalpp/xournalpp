#include "HomeWindow.h"

#include <regex>

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_new_fr...
#include <gdk/gdk.h>                // for gdk_screen_get_de...
#include <gio/gio.h>                // for g_cancellable_is_...
#include <gtk/gtkcssprovider.h>     // for gtk_css_provider_...

#include "control/AudioController.h"                    // for AudioController
#include "control/Control.h"                            // for Control
#include "control/DeviceListHelper.h"                   // for getSourceMapping
#include "control/ScrollHandler.h"                      // for ScrollHandler
#include "control/actions/ActionDatabase.h"             // for ActionDatabase
#include "control/jobs/XournalScheduler.h"              // for XournalScheduler
#include "control/layer/LayerController.h"              // for LayerController
#include "control/settings/Settings.h"                  // for Settings
#include "control/settings/SettingsEnums.h"             // for SCROLLBAR_HIDE_HO...
#include "control/zoom/ZoomControl.h"                   // for ZoomControl
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
#include "gui/Builder.h"

#include "GladeSearchpath.h"     // for GladeSearchpath
#include "ToolbarDefinitions.h"  // for TOOLBAR_DEFINITIO...
#include "XournalView.h"         // for XournalView
#include "config-dev.h"          // for TOOLBAR_CONFIG
#include "filesystem.h"          // for path, exists                            // for GladeGui

#include <iostream>

HomeWindow::HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app)
    : GladeGui(gladeSearchPath, "homepage.glade", "windowhome"), control(control) {
   gtk_window_set_application(GTK_WINDOW(getWindow()), app);
   // if (!getWindow()) {
     //   g_error("Failed to initialize HomeWindow: window is null");
    //}

    // Connect the button signals
    getWindow();
    initHomeWidget();
    
    Builder builder(gladeSearchPath, "homepage.glade");
    g_signal_connect_swapped(GTK_BUTTON(builder.get("start_button")), "clicked", G_CALLBACK(+[](HomeWindow* self){self->on_button_click_me_clicked(); }), this);
    std::cout << "HomeWindow created\n";
    //GtkWidget* button = GTK_WIDGET(get("button_click_me"));
    //g_signal_connect(button, "clicked", G_CALLBACK(on_button_click_me_clicked), this);
}


HomeWindow::~HomeWindow() = default;

void HomeWindow::initHomeWidget() {

    //no hace ningun efecto dado que parece no tener efecto en el .glade
    /* gtk_window_set_title(GTK_WINDOW(winHome), "Home");
    gtk_window_set_default_size(GTK_WINDOW(winHome), 1600, 800); */

    winHome = gtk_window_new(GTK_WINDOW_TOPLEVEL); // no hay forma de verificar si funciona
    gtk_window_set_position(GTK_WINDOW(winHome), GTK_WIN_POS_CENTER); // parece que no funciona

    //GtkWidget* vpMain = gtk_viewport_new(nullptr, nullptr);


   // GtkWidget* vpMain = gtk_viewport_new(nullptr, nullptr);

//    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(winHome), vpMain);

  //  scrollHandling = std::make_unique<ScrollHandling>(GTK_SCROLLABLE(vpMain));

    //this->home = std::make_unique<HomeView>(vpMain, control, scrollHandling.get());

    //control->getZoomControl()->initZoomHandler(this->window, winHome, home.get(), control);

}

void HomeWindow::show(GtkWindow* parent) { gtk_widget_show(this->window); }

void HomeWindow::on_button_click_me_clicked() { //se quitaron los parametros para prueba
  std::cout << "Button clicked!\n";
  g_print("Button clicked!\n");
}