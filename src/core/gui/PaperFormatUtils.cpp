#include "PaperFormatUtils.h"

#include "util/GListView.h"
#include "util/StringUtils.h"
#include "util/raii/GObjectSPtr.h"

using xoj::util::GtkPaperSizeUPtr;

void PaperFormatUtils::loadDefaultPaperSizes(PaperFormatMenuOptionVector& paperSizes) {
    GList* default_sizes = gtk_paper_size_get_paper_sizes(false);
    for (auto& s: GListView<GtkPaperSize>(default_sizes)) {
        std::string name = gtk_paper_size_get_name(&s);
        if (name == GTK_PAPER_NAME_A3 || name == GTK_PAPER_NAME_A4 || name == GTK_PAPER_NAME_A5 ||
            name == GTK_PAPER_NAME_LETTER || name == GTK_PAPER_NAME_LEGAL) {
            paperSizes.emplace_back(GtkPaperSizeUPtr(&s));
        } else {
            gtk_paper_size_free(&s);
        }
    }
    g_list_free(default_sizes);

    // Name format: ftp://ftp.pwg.org/pub/pwg/candidates/cs-pwgmsn10-20020226-5101.1.pdf
    paperSizes.emplace_back(GtkPaperSizeUPtr(gtk_paper_size_new("custom_16x9_320x180mm")));
    paperSizes.emplace_back(GtkPaperSizeUPtr(gtk_paper_size_new("custom_4x3_320x240mm")));
}
void PaperFormatUtils::fillPaperFormatDropDown(const PaperFormatMenuOptionVector& menuOptions,
                                               GtkComboBox* paperFormatComboBox) {
    // Todo (gtk4) Use GtkDropDown instead and use another model accordingly
    GtkCellRenderer* cell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(paperFormatComboBox), cell, true);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(paperFormatComboBox), cell, "text", 0, nullptr);

    GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
    gtk_combo_box_set_model(GTK_COMBO_BOX(paperFormatComboBox), GTK_TREE_MODEL(store));
    g_object_unref(store);  // store is now owned by paperFormatComboBox

    for (auto& menuOption: menuOptions) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);

        if (menuOption.index()) {
            auto& size = std::get<GtkPaperSizeUPtr>(menuOption);

            std::string displayName = gtk_paper_size_get_display_name(size.get());
            if (StringUtils::startsWith(displayName, "custom_")) {
                displayName = displayName.substr(7);
            }
            gtk_list_store_set(store, &iter, 0, displayName.c_str(), -1);
            gtk_list_store_set(store, &iter, 1, size.get(), -1);
        } else {
            // When special non-GtkPaperSize option
            const std::string_view displayName = std::get<std::string>(menuOption);
            gtk_list_store_set(store, &iter, 0, displayName.data(), -1);
            gtk_list_store_set(store, &iter, 1, nullptr, -1);
        }
    }
}
