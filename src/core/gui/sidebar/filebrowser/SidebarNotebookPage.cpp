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

    this->treeStore = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
    this->treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(this->treeStore));
    g_object_unref(this->treeStore); // TreeView holds reference now

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(this->treeView), false);

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COLUMN_NAME, nullptr);
    gtk_tree_view_append_column(GTK_TREE_VIEW(this->treeView), column);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(this->scrolledWindow), this->treeView);

    g_signal_connect(this->treeView, "row-activated", G_CALLBACK(onRowActivated), this);

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
    gtk_tree_store_clear(this->treeStore);

    fs::path notebookFolder = this->control->getSettings()->getNotebookFolder();
    if (notebookFolder.empty() || !fs::is_directory(notebookFolder)) {
        return;
    }

    try {
        std::vector<fs::path> subfolders;
        for (const auto& entry : fs::directory_iterator(notebookFolder)) {
            if (entry.is_directory()) {
                subfolders.push_back(entry.path());
            }
        }
        std::sort(subfolders.begin(), subfolders.end());

        for (const auto& subfolder : subfolders) {
            GtkTreeIter parentIter;
            gtk_tree_store_append(this->treeStore, &parentIter, nullptr);
            gtk_tree_store_set(this->treeStore, &parentIter, COLUMN_NAME, subfolder.filename().string().c_str(), COLUMN_FILEPATH, "", -1);

            std::vector<fs::path> files;
            for (const auto& fileEntry : fs::directory_iterator(subfolder)) {
                if (fileEntry.is_regular_file() && fileEntry.path().extension() == ".xopp") {
                    files.push_back(fileEntry.path());
                }
            }
            std::sort(files.begin(), files.end());

            for (const auto& file : files) {
                GtkTreeIter childIter;
                gtk_tree_store_append(this->treeStore, &childIter, &parentIter);
                gtk_tree_store_set(this->treeStore, &childIter, COLUMN_NAME, file.filename().string().c_str(), COLUMN_FILEPATH, file.string().c_str(), -1);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Log or simply ignore restricted directories
        g_warning("Could not read notebook folder: %s", e.what());
    }
}

void SidebarNotebookPage::onRowActivated(GtkTreeView* treeView, GtkTreePath* path, GtkTreeViewColumn* column, SidebarNotebookPage* sidebar) {
    GtkTreeIter iter;
    GtkTreeModel* model = gtk_tree_view_get_model(treeView);

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gchar* filepath = nullptr;
        gtk_tree_model_get(model, &iter, COLUMN_FILEPATH, &filepath, -1);

        if (filepath && strlen(filepath) > 0) {
            fs::path p(filepath);
            sidebar->control->openFile(p, [](bool) {}, -1, false);
        }
        g_free(filepath);
    }
}

void SidebarNotebookPage::onNewNotebook(GtkButton* button, SidebarNotebookPage* sidebar) {
    fs::path notebookFolder = sidebar->control->getSettings()->getNotebookFolder();
    if (notebookFolder.empty()) {
        sidebar->control->newFile();
        return;
    }

    // Determine a suggested path for the new notebook
    fs::path suggestedPath = notebookFolder / fs::path((const char8_t*)sidebar->control->getSettings()->getDefaultSaveName().c_str());

    xoj::SaveExportDialog::showSaveFileDialog(
            sidebar->control->getGtkWindow(), sidebar->control->getSettings(), std::move(suggestedPath),
            [sidebar](std::optional<fs::path> p) {
                if (p && !p->empty()) {
                    sidebar->control->getSettings()->setLastSavePath(p->parent_path());
                    sidebar->control->newFile(p.value());
                }
            });
}
