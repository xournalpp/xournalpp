#include "PageTypeSelectionPopover.h"

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <glib.h>
#include <gtk/gtkactionable.h>

#include "control/PageBackgroundChangeController.h"
#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "control/settings/Settings.h"
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
static constexpr auto PAGETYPE_SELECTION_ACTION_NAME = "select-page-type-of-new-page";
static constexpr auto ORIENTATION_SELECTION_ACTION_NAME = "select-page-orientation-of-new-page";

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
auto getInitiallySelectedPaperSize(const Settings* settings) -> std::optional<PaperSize> {
    if (settings) {
        PageTemplateSettings model;
        model.parse(settings->getPageTemplate());
        return model.isCopyLastPageSize() ? std::nullopt : std::optional(PaperSize(model));
    }
    return std::nullopt;
}
auto createOrientationGAction(uint8_t orientation) -> GSimpleAction* {
    return g_simple_action_new_stateful(ORIENTATION_SELECTION_ACTION_NAME, G_VARIANT_TYPE_BOOLEAN,
                                        g_variant_new_boolean(orientation));
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
        PageTypeSelectionMenuBase(typesHandler, settings, PAGETYPE_SELECTION_ACTION_NAME),
        controller(controller),
        settings(settings),
        selectedPageSize(getInitiallySelectedPaperSize(settings)),
        selectedOrientation(selectedPageSize->orientation()) {
    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));

    PaperFormatUtils::loadDefaultPaperSizes(paperSizeMenuOptions);
    // Add Special options
    customPaperSizeIndex = paperSizeMenuOptions.size();
    paperSizeMenuOptions.emplace_back(_("Custom"));
    copyCurrentPaperSizeIndex = paperSizeMenuOptions.size();
    paperSizeMenuOptions.emplace_back(_("Copy current page"));

    this->controller->setPaperSizeForNewPages(this->selectedPageSize);

    orientationAction.reset(createOrientationGAction(selectedOrientation), xoj::util::adopt);
    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(orientationAction.get()));
    g_simple_action_set_enabled(orientationAction.get(), selectedPageSize.has_value());

    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(typeSelectionAction.get()));
}

unsigned int PageTypeSelectionPopover::getComboBoxIndexForPaperSize(const std::optional<PaperSize>& paperSize) const {
    if (!paperSize) {
        return copyCurrentPaperSizeIndex;
    }

    for (int i = 0; i < customPaperSizeIndex; i++) {
        auto& currentPaperSize = std::get<PaperFormatUtils::GtkPaperSizeUniquePtr_t>(paperSizeMenuOptions[i]);
        const PaperSize aDefaultPaperSize(currentPaperSize);
        if (paperSize->equalDimensions(aDefaultPaperSize)) {
            return i;
        }
    }
    return customPaperSizeIndex;  // Custom option is returned if no matching format is found
}
GtkWidget* PageTypeSelectionPopover::createOrientationButton(std::string_view actionName, GtkOrientation orientation,
                                                             std::string_view icon) const {
    GtkWidget* button = gtk_toggle_button_new();
    gtk_container_add(GTK_CONTAINER(button), gtk_image_new_from_icon_name(icon.data(), GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(button), g_variant_new_boolean(orientation));
    return button;
}
GtkWidget* PageTypeSelectionPopover::createPopover() const {
    GtkWidget* popover = gtk_popover_new();

    // Todo(cpp20): constexpr this
    std::string prefixedPageTypeActionName = G_ACTION_NAMESPACE;
    prefixedPageTypeActionName += PAGETYPE_SELECTION_ACTION_NAME;
    std::string prefixedOrientationActionName = std::string(G_ACTION_NAMESPACE) + ORIENTATION_SELECTION_ACTION_NAME;

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_popover_set_child(GTK_POPOVER(popover), GTK_WIDGET(box));

    gtk_box_append(box, createPreviewGrid(types->getPageTypes(), prefixedPageTypeActionName));
    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkGrid* grid = createEmptyGrid();
    int gridX = 0;
    // Create a special entry for copying the current page's background
    // It has index == npos
    gtk_grid_attach(grid, createEntryWithoutPreview(_("Copy current background"), npos, prefixedPageTypeActionName),
                    gridX++, 0, 1, 1);

    // The indices of special page types start after the normal page types'
    size_t n = types->getPageTypes().size();
    for (const auto& pageInfo: types->getSpecialPageTypes()) {
        gtk_grid_attach(grid, createEntryWithoutPreview(pageInfo->name.c_str(), n++, prefixedPageTypeActionName),
                        gridX++, 0, 1, 1);
    }

    while (gridX < PAGE_TYPES_PER_ROW) {
        // Add empty cells to the grid so the buttons don't get extended
        gtk_grid_attach(grid, gtk_label_new(""), gridX++, 0, 1, 1);
    }

    GtkBox* pageFormatBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    gtk_widget_set_margin_start(GTK_WIDGET(pageFormatBox), 20);
    gtk_widget_set_margin_end(GTK_WIDGET(pageFormatBox), 20);
    gtk_widget_set_margin_top(GTK_WIDGET(pageFormatBox), 20);
    gtk_widget_set_margin_bottom(GTK_WIDGET(pageFormatBox), 6);

    GtkBox* orientationFormatBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(orientationFormatBox));
    gtk_style_context_add_class(context, "linked");

    std::array<GtkWidget*, 2> orientationButtons{};
    orientationButtons[GTK_ORIENTATION_VERTICAL] = createOrientationButton(
            prefixedOrientationActionName, GTK_ORIENTATION_VERTICAL, "xopp-orientation-portrait");
    orientationButtons[GTK_ORIENTATION_HORIZONTAL] = createOrientationButton(
            prefixedOrientationActionName, GTK_ORIENTATION_HORIZONTAL, "xopp-orientation-landscape");

    gtk_box_append(orientationFormatBox, orientationButtons[GTK_ORIENTATION_VERTICAL]);
    gtk_box_append(orientationFormatBox, orientationButtons[GTK_ORIENTATION_HORIZONTAL]);

    gtk_box_append(pageFormatBox, GTK_WIDGET(orientationFormatBox));

    GtkComboBox* pageFormatComboBox = PaperFormatUtils::createPaperFormatDropDown(paperSizeMenuOptions);
    gtk_combo_box_set_active(GTK_COMBO_BOX(pageFormatComboBox), getComboBoxIndexForPaperSize(selectedPageSize));
    gtk_box_append(pageFormatBox, GTK_WIDGET(pageFormatComboBox));

    gtk_box_append(box, GTK_WIDGET(grid));
    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    gtk_box_append(box, GTK_WIDGET(pageFormatBox));

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
    gtk_widget_set_sensitive(applyToCurrentPageButton,
                             this->selectedPT.has_value() || this->selectedPageSize.has_value());
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

    gtk_widget_show_all(GTK_WIDGET(box));

    return popover;
}

void PageTypeSelectionPopover::entrySelected(const PageTypeInfo*) {
    this->controller->setPageTypeForNewPages(this->selectedPT);
}
