#include "SidebarNotebookPage.h"

#include <filesystem>
#include <vector>
#include <algorithm>

#include <glib.h>

#include "control/Control.h"
#include "control/settings/Settings.h"
#include "util/PathUtil.h"
#include "util/i18n.h"
#include "util/gtk4_helper.h"
#include "util/glib_casts.h"
#include "gui/dialog/XojSaveDlg.h"
#include "model/Document.h"

SidebarNotebookPage::SidebarNotebookPage(Control* control):
        AbstractSidebarPage(control), iconNameHelper(control->getSettings()) {

    this->mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkStyleContext* context = gtk_widget_get_style_context(toolbar);
    gtk_style_context_add_class(context, "toolbar");
    gtk_style_context_add_class(context, "linked");

    GtkWidget* newBtn = gtk_button_new_from_icon_name("document-new");
    gtk_widget_set_tooltip_text(newBtn, _("New Notebook"));
    g_signal_connect(newBtn, "clicked", G_CALLBACK(onNewNotebook), this);
    gtk_box_append(GTK_BOX(toolbar), newBtn);

    gtk_box_append(GTK_BOX(this->mainBox), toolbar);

    this->scrolledWindow = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scrolledWindow), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(this->scrolledWindow, true);

    this->listBox = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(this->scrolledWindow), this->listBox);

    g_signal_connect(this->listBox, "row-activated", G_CALLBACK(onRowActivated), this);

    gtk_box_append(GTK_BOX(this->mainBox), this->scrolledWindow);

    updateList();

    gtk_widget_show_all(this->mainBox);
}

SidebarNotebookPage::~SidebarNotebookPage() = default;

void SidebarNotebookPage::enableSidebar() {
    updateList();
}

void SidebarNotebookPage::disableSidebar() {}

void SidebarNotebookPage::layout() {}

std::string SidebarNotebookPage::getName() { return _("Notebooks"); }

std::string SidebarNotebookPage::getIconName() { return this->iconNameHelper.iconName("sidebar-notebook"); }

bool SidebarNotebookPage::hasData() { return true; }

GtkWidget* SidebarNotebookPage::getWidget() { return this->mainBox; }

void SidebarNotebookPage::updateList() {
    // Clear list box
    gtk_container_foreach(
            GTK_CONTAINER(this->listBox),
            +[](GtkWidget* child, gpointer data) {
                gtk_container_remove(GTK_CONTAINER(data), child);
            },
            this->listBox);

    fs::path notebookFolder = this->control->getSettings()->getNotebookFolder();
    if (notebookFolder.empty() || !fs::is_directory(notebookFolder)) {
        return;
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(notebookFolder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".xopp") {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());

    for (const auto& path : files) {
        GtkWidget* row = gtk_list_box_row_new();
        g_object_set_data_full(G_OBJECT(row), "filepath", g_strdup(path.string().c_str()), g_free);
        GtkWidget* label = gtk_label_new(path.filename().string().c_str());
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_widget_set_margin_top(label, 5);
        gtk_widget_set_margin_bottom(label, 5);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        gtk_list_box_append(GTK_LIST_BOX(this->listBox), row);
    }
}

void SidebarNotebookPage::onRowActivated(GtkListBox* box, GtkListBoxRow* row, SidebarNotebookPage* sidebar) {
    const char* pathStr = static_cast<const char*>(g_object_get_data(G_OBJECT(row), "filepath"));
    if (pathStr) {
        fs::path path(pathStr);
        sidebar->control->openFile(path, [](bool) {}, -1, false);
    }
}

void SidebarNotebookPage::onNewNotebook(GtkButton* button, SidebarNotebookPage* sidebar) {
    fs::path notebookFolder = sidebar->control->getSettings()->getNotebookFolder();
    if (notebookFolder.empty()) {
        sidebar->control->newFile();
        return;
    }

    // Determine a suggested path for the new notebook
    fs::path suggestedPath = notebookFolder / sidebar->control->getDocument()->createSaveFilename(Document::XOPP, sidebar->control->getSettings()->getDefaultSaveName());

    xoj::SaveExportDialog::showSaveFileDialog(
            sidebar->control->getGtkWindow(), sidebar->control->getSettings(), std::move(suggestedPath),
            [sidebar](std::optional<fs::path> p) {
                if (p && !p->empty()) {
                    sidebar->control->getSettings()->setLastSavePath(p->parent_path());
                    sidebar->control->newFile(p.value());
                }
            });
}
