#include "PageTypeSelectionPopover.h"

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <glib.h>
#include <gtk/gtkactionable.h>

#include "control/PageBackgroundChangeController.h"
#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "gui/CreatePreviewImage.h"
#include "gui/menus/StaticAssertActionNamespace.h"
#include "model/PageType.h"  // for PageType
#include "util/Assert.h"
#include "util/GListView.h"
#include "util/GtkUtil.h"
#include "util/Util.h"
#include "util/gtk4_helper.h"  // for gtk_box_append
#include "util/i18n.h"         // for _
#include "util/raii/GObjectSPtr.h"
#include "util/raii/GVariantSPtr.h"


namespace {
static constexpr auto PAGE_TYPES_PER_ROW = 4;
static constexpr auto G_ACTION_NAMESPACE = "win.";
static constexpr auto SELECTION_ACTION_NAME = "select-page-type-of-new-page";

GtkWidget* createEntryWithoutPreview(const char* label, size_t entryNb, const std::string_view& prefixedActionName) {
    GtkWidget* button = gtk_toggle_button_new();

    GtkWidget* labelWidget = gtk_label_new(label);
    gtk_label_set_wrap_mode(GTK_LABEL(labelWidget), PANGO_WRAP_WORD);
    gtk_label_set_wrap(GTK_LABEL(labelWidget), true);
    gtk_label_set_justify(GTK_LABEL(labelWidget), GTK_JUSTIFY_CENTER);
    gtk_label_set_max_width_chars(GTK_LABEL(labelWidget), 1);
    gtk_widget_set_size_request(labelWidget, xoj::helper::PREVIEW_WIDTH, xoj::helper::PREVIEW_HEIGHT);

    gtk_button_set_child(GTK_BUTTON(button), labelWidget);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), prefixedActionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(button), xoj::util::makeGVariantSPtr(entryNb).get());
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(button));
    return button;
}

GtkGrid* createEmptyGrid() {
    GtkGrid* grid = GTK_GRID(gtk_grid_new());

    gtk_grid_set_column_homogeneous(grid, true);
    gtk_grid_set_row_homogeneous(grid, true);
    gtk_grid_set_column_spacing(grid, 10);
    gtk_grid_set_row_spacing(grid, 10);
    gtk_widget_set_margin_start(GTK_WIDGET(grid), 10);
    gtk_widget_set_margin_end(GTK_WIDGET(grid), 10);

    return grid;
}

/**
 * @brief Create a togglebutton containing a miniature of the given (standard) page type
 *      The toggle button is initialized to follow a GAction with the given action name and target value entryNb.
 *      The returned widget is a floating ref.
 */
GtkWidget* createEntryWithPreview(const PageTypeInfo* pti, size_t entryNb, const std::string_view& prefixedActionName) {
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
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(button));
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
};  // namespace

PageTypeSelectionPopover::PageTypeSelectionPopover(PageTypeHandler* typesHandler,
                                                   PageBackgroundChangeController* controller, const Settings* settings,
                                                   GtkApplicationWindow* win):
        PageTypeSelectionMenuBase(typesHandler, settings, SELECTION_ACTION_NAME), controller(controller) {
    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));

    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(typeSelectionAction.get()));

    controller->setPageTypeForNewPages(this->selectedPT);
}

GtkWidget* PageTypeSelectionPopover::createPopover() const {
    GtkWidget* popover = gtk_popover_new();

    // Todo(cpp20): constexpr this
    std::string prefixedActionName = G_ACTION_NAMESPACE;
    prefixedActionName += SELECTION_ACTION_NAME;

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_popover_set_child(GTK_POPOVER(popover), GTK_WIDGET(box));

    gtk_box_append(box, createPreviewGrid(types->getPageTypes(), prefixedActionName));
    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkGrid* grid = createEmptyGrid();
    int gridX = 0;
    // Create a special entry for copying the current page's background
    // It has index == npos
    gtk_grid_attach(grid, createEntryWithoutPreview(_("Copy current background"), npos, prefixedActionName), gridX++, 0,
                    1, 1);

    // The indices of special page types start after the normal page types'
    size_t n = types->getPageTypes().size();
    for (const auto& pageInfo: types->getSpecialPageTypes()) {
        gtk_grid_attach(grid, createEntryWithoutPreview(pageInfo->name.c_str(), n++, prefixedActionName), gridX++, 0, 1,
                        1);
    }

    while (gridX < PAGE_TYPES_PER_ROW) {
        // Add empty cells to the grid so the buttons don't get extended
        gtk_grid_attach(grid, gtk_label_new(""), gridX++, 0, 1, 1);
    }

    gtk_box_append(box, GTK_WIDGET(grid));
    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkWidget* applyToCurrentPageButton = gtk_button_new_with_label(_("Apply to current page"));
    g_signal_connect(applyToCurrentPageButton, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer d) {
                         auto self = static_cast<const PageTypeSelectionPopover*>(d);
                         if (self->selectedPT) {
                             self->controller->changeCurrentPageBackground(self->selectedPT.value());
                         }
                     }),
                     const_cast<PageTypeSelectionPopover*>(this));
    gtk_box_append(box, applyToCurrentPageButton);

    // We cannot "Apply to current page" if no page type is selected...
    gtk_widget_set_sensitive(applyToCurrentPageButton, this->selectedPT.has_value());
    g_signal_connect_object(this->typeSelectionAction.get(), "notify::state",
                            G_CALLBACK(+[](GObject* a, GParamSpec*, gpointer btn) {
                                xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
                                gtk_widget_set_sensitive(GTK_WIDGET(btn), getGVariantValue<size_t>(state.get()) !=
                                                                                  COPY_CURRENT_PLACEHOLDER);
                            }),
                            applyToCurrentPageButton, GConnectFlags(0));

    GtkWidget* button = gtk_button_new_with_label(_("Apply to all pages"));
    g_signal_connect(button, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer d) {
                         auto self = static_cast<const PageTypeSelectionPopover*>(d);
                         if (self->selectedPT) {
                             self->controller->applyBackgroundToAllPages(self->selectedPT.value());
                         } else {
                             self->controller->applyCurrentPageBackgroundToAll();
                         }
                     }),
                     const_cast<PageTypeSelectionPopover*>(this));
    gtk_box_append(box, button);

    return popover;
}

void PageTypeSelectionPopover::entrySelected(const PageTypeInfo*) {
    this->controller->setPageTypeForNewPages(this->selectedPT);
}
