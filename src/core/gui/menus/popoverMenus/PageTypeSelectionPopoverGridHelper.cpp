#include "PageTypeSelectionPopoverGridHelper.h"

#include <glib.h>

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "gui/CreatePreviewImage.h"
#include "model/PageType.h"  // for PageType
#include "util/Assert.h"
#include "util/raii/GVariantSPtr.h"

namespace xoj::helper::PageTypeGrid {
GtkGrid* createEmptyGrid() {
    GtkGrid* grid = GTK_GRID(gtk_grid_new());

    gtk_grid_set_column_homogeneous(grid, true);
    gtk_grid_set_row_homogeneous(grid, true);
    gtk_grid_set_column_spacing(grid, 10);
    gtk_grid_set_row_spacing(grid, 10);
    gtk_widget_add_css_class(GTK_WIDGET(grid), "page-type-grid");

    return grid;
}

/**
 * @brief Create a togglebutton containing a miniature of the given (standard) page type
 *      The toggle button is initialized to follow a GAction with the given action name and target value entryNb.
 *      The returned widget is a floating ref.
 */
static GtkWidget* createEntryWithPreview(const PageTypeInfo* pti, size_t entryNb,
                                         const std::string_view& prefixedActionName) {
    GtkWidget* button = gtk_toggle_button_new();

    // // Use to restore labels in the menu
    // GtkWidget* label = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    // gtk_box_append(GTK_BOX(label), createPreviewImage(*pti));
    // gtk_box_append(GTK_BOX(label), gtk_label_new(pti->name.c_str()));
    // gtk_button_set_child(GTK_BUTTON(button), label);

    GtkWidget* preview = xoj::helper::createPreviewImage(pti->page);
    gtk_widget_set_tooltip_text(preview, pti->name.c_str());
    gtk_button_set_child(GTK_BUTTON(button), preview);  // takes ownership of preview
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), prefixedActionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(button), xoj::util::makeGVariantSPtr(entryNb).get());
    return button;
}

/**
 * @brief Create a grid containing a miniature for each one the given (standard) page types
 *      The returned widget is a floating ref.
 */
GtkWidget* createPreviewGrid(const std::vector<std::unique_ptr<PageTypeInfo>>& pageTypes,
                             const std::string_view& prefixedActionName) {
    GtkGrid* grid = createEmptyGrid();
    gtk_widget_set_margin_top(GTK_WIDGET(grid), 10);

    size_t n = 0;
    int gridX = 0;
    int gridY = 0;

    for (const auto& pageInfo: pageTypes) {
        // Special page types do not get a preview
        xoj_assert(!pageInfo->page.isSpecial());
        auto* entry = createEntryWithPreview(pageInfo.get(), n++, prefixedActionName);
        if (gridX >= PAGE_TYPES_PER_ROW) {
            gridX = 0;
            gridY++;
        }
        gtk_grid_attach(grid, entry, gridX, gridY, 1, 1);
        gridX++;
    }
    return GTK_WIDGET(grid);
}
};  // namespace xoj::helper::PageTypeGrid
