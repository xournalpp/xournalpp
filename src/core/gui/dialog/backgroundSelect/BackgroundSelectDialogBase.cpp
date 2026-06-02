#include "BackgroundSelectDialogBase.h"

#include <algorithm>  // for max
#include <utility>    // for move

#include <gdk/gdk.h>      // for GDK_EXPOSURE_MASK
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "gui/Builder.h"
#include "util/Util.h"        // for paintBackgroundWhite
#include "util/safe_casts.h"  // for round_cast

#include "BaseElementView.h"  // for BaseElementView

class GladeSearchpath;


constexpr auto UI_FILE = "backgroundSelection.ui";
constexpr auto UI_DIALOG_NAME = "Dialog";

BackgroundSelectDialogBase::BackgroundSelectDialogBase(GladeSearchpath* gladeSearchPath, Document* doc,
                                                       Settings* settings, const char* title):
        settings(settings), doc(doc) {
    Builder builder(gladeSearchPath, UI_FILE);
    this->window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));
    gtk_window_set_title(window.get(), title);

    this->container = GTK_FIXED(gtk_fixed_new());
    gtk_widget_set_hexpand(GTK_WIDGET(this->container), true);
    this->okButton = builder.get("btOk");
    this->vbox = GTK_BOX(builder.get("vbox"));

    this->scrolledWindow = GTK_SCROLLED_WINDOW(builder.get("scrollContents"));
    gtk_scrolled_window_set_child(scrolledWindow, GTK_WIDGET(container));

    g_signal_connect(gtk_scrolled_window_get_hadjustment(scrolledWindow), "notify::page-size",
                     G_CALLBACK(+[](GObject*, GParamSpec*, gpointer d) {
                         static_cast<BackgroundSelectDialogBase*>(d)->layout();
                     }),
                     this);

    gtk_window_set_default_size(this->window.get(), 800, 600);
    gtk_widget_set_sensitive(this->okButton, false);

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());

    g_signal_connect(this->container, "realize", G_CALLBACK(+[](GtkWidget*, gpointer self) {
                         static_cast<BackgroundSelectDialogBase*>(self)->layout();
                     }),
                     this);
}

BackgroundSelectDialogBase::~BackgroundSelectDialogBase() = default;

auto BackgroundSelectDialogBase::getSettings() -> Settings* { return this->settings; }

void BackgroundSelectDialogBase::layout() {
    int x = 0;
    int y = 0;
    int row_height = 0;
    int max_row_width = 0;

    auto width = gtk_adjustment_get_page_size(gtk_scrolled_window_get_hadjustment(scrolledWindow));

    for (const auto& p: this->entries) {
        if (!gtk_widget_get_visible(p->getWidget())) {
            continue;
        }

        if (x + p->getWidth() > width) {
            max_row_width = std::max(max_row_width, x);
            y += row_height;
            x = 0;
            row_height = 0;
        }
        gtk_fixed_move(this->container, p->getWidget(), x, y);

        row_height = std::max(row_height, p->getHeight());
        x += p->getWidth();
    }
    max_row_width = std::max(max_row_width, x);

    gtk_widget_set_size_request(GTK_WIDGET(this->container), max_row_width, y + row_height);
}

void BackgroundSelectDialogBase::populate() {
    for (const auto& e: entries) {
        gtk_fixed_put(this->container, e->getWidget(), 0, 0);
    }

    if (!entries.empty()) {
        setSelected(0);
    }

    layout();
}

void BackgroundSelectDialogBase::setSelected(size_t selected) {
    if (this->selected == selected) {
        return;
    }

    size_t lastSelected = this->selected;
    if (lastSelected < entries.size()) {
        entries[lastSelected]->setSelected(false);
    }

    if (selected < entries.size()) {
        entries[selected]->setSelected(true);
        this->selected = selected;
        gtk_widget_set_sensitive(okButton, true);
    }
}
