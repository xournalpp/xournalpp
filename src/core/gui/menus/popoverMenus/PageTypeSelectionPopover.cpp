#include "PageTypeSelectionPopover.h"

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <glib.h>
#include <gtk/gtkactionable.h>

#include "control/PageBackgroundChangeController.h"
#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "control/settings/Settings.h"
#include "gui/CreatePreviewImage.h"
#include "gui/dialog/FormatDialog.h"
#include "gui/menus/StaticAssertActionNamespace.h"
#include "model/XojPage.h"
#include "util/Assert.h"
#include "util/PopupWindowWrapper.h"
#include "util/Util.h"
#include "util/i18n.h"         // for _
#include "util/raii/GVariantSPtr.h"

#include "PageTypeSelectionPopoverGridHelper.h"


namespace {
static constexpr auto G_ACTION_NAMESPACE = "win.";
static constexpr auto PAGETYPE_SELECTION_ACTION_NAME = "select-page-type-of-new-page";
static constexpr auto ORIENTATION_SELECTION_ACTION_NAME = "select-page-orientation-of-new-page";
static constexpr auto PAGESIZE_COMBOBOX_CHANGE_ACTION_NAME = "change-combobox-page-size-selection-new-page";
static constexpr auto PAGESIZE_CHANGED_ACTION_NAME = "changed-page-size-selection-new-page";

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
    return button;
}

auto getInitiallySelectedPaperSize(const Settings* settings) -> std::optional<PaperSize> {
    if (settings) {
        PageTemplateSettings model;
        model.parse(settings->getPageTemplate());
        return model.isCopyLastPageSize() ? std::nullopt : std::optional(PaperSize(model));
    }
    return std::nullopt;
}
auto createOrientationGAction(PaperOrientation orientation) -> GSimpleAction* {
    return g_simple_action_new_stateful(ORIENTATION_SELECTION_ACTION_NAME, G_VARIANT_TYPE_BOOLEAN,
                                        g_variant_new_boolean(static_cast<bool>(orientation)));
}
auto createPageSizeComboBoxChangeSelectionGAction() -> GSimpleAction* {
    return g_simple_action_new(PAGESIZE_COMBOBOX_CHANGE_ACTION_NAME, G_VARIANT_TYPE_UINT32);
}
auto createPageSizeChangedGAction() -> GSimpleAction* {
    return g_simple_action_new(PAGESIZE_CHANGED_ACTION_NAME, nullptr);
}

GtkWidget* createOrientationButton(std::string_view actionName, GtkOrientation orientation, std::string_view icon) {
    GtkWidget* button = gtk_toggle_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(button), icon.data());
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(button), g_variant_new_boolean(orientation));
    return button;
}

/**
 * @brief Create a separator with custom margins
 */
GtkWidget* createSeparator() {
    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_start(separator, 7);
    gtk_widget_set_margin_end(separator, 7);
    return separator;
}
};  // namespace

PageTypeSelectionPopover::PageTypeSelectionPopover(Control* control, Settings* settings, GtkApplicationWindow* win):
        PageTypeSelectionMenuBase(control->getPageTypes(), settings, PAGETYPE_SELECTION_ACTION_NAME),
        control(control),
        controller(control->getPageBackgroundChangeController()),
        settings(settings),
        selectedPageSize(getInitiallySelectedPaperSize(settings)),
        selectedOrientation(selectedPageSize ? selectedPageSize->orientation() : PaperOrientation::VERTICAL) {
    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));

    PaperFormatUtils::loadDefaultPaperSizes(paperSizeMenuOptions);
    // Add Special options
    customPaperSizeIndex = static_cast<uint32_t>(paperSizeMenuOptions.size());
    paperSizeMenuOptions.emplace_back(_("Custom"));
    copyCurrentPaperSizeIndex = static_cast<uint32_t>(paperSizeMenuOptions.size());
    paperSizeMenuOptions.emplace_back(_("Copy current page"));

    this->controller->setPaperSizeForNewPages(this->selectedPageSize);

    orientationAction.reset(createOrientationGAction(selectedOrientation), xoj::util::adopt);
    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(orientationAction.get()));
    g_simple_action_set_enabled(orientationAction.get(), selectedPageSize.has_value());
    g_signal_connect(G_OBJECT(orientationAction.get()), "change-state", G_CALLBACK(changedOrientationSelectionCallback),
                     this);

    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(typeSelectionAction.get()));

    comboBoxChangeSelectionAction.reset(createPageSizeComboBoxChangeSelectionGAction(), xoj::util::adopt);
    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(comboBoxChangeSelectionAction.get()));

    pageSizeChangedAction.reset(createPageSizeChangedGAction(), xoj::util::adopt);
    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(pageSizeChangedAction.get()));
}
template <bool changeComboBoxSelection>
void PageTypeSelectionPopover::setSelectedPaperSize(const std::optional<PaperSize>& newPageSize) {
    if (newPageSize != selectedPageSize) {
        if constexpr (changeComboBoxSelection) {
            ignoreComboBoxSelectionChange = true;
            g_action_activate(G_ACTION(comboBoxChangeSelectionAction.get()),
                              g_variant_new_uint32(static_cast<uint32_t>(getComboBoxIndexForPaperSize(newPageSize))));
            ignoreComboBoxSelectionChange = false;
        }

        selectedPageSize = newPageSize;
        if (selectedPageSize) {
            g_simple_action_set_state(orientationAction.get(),
                                      g_variant_new_boolean(static_cast<bool>(selectedPageSize->orientation())));
        }
        g_simple_action_set_enabled(orientationAction.get(), selectedPageSize.has_value());

        // Updates sensitivity of 'apply to current'-button
        g_action_activate(G_ACTION(this->pageSizeChangedAction.get()), nullptr);

        controller->setPaperSizeForNewPages(selectedPageSize);
    }
}
// Explicit instantiation of the two possible bool values to avoid having to put the function template into header
template void PageTypeSelectionPopover::setSelectedPaperSize<true>(const std::optional<PaperSize>& newPageSize);
template void PageTypeSelectionPopover::setSelectedPaperSize<false>(const std::optional<PaperSize>& newPageSize);

unsigned int PageTypeSelectionPopover::getComboBoxIndexForPaperSize(const std::optional<PaperSize>& paperSize) const {
    if (!paperSize) {
        return copyCurrentPaperSizeIndex;
    }

    for (unsigned int i = 0; i < customPaperSizeIndex; i++) {
        auto& currentPaperSize = std::get<xoj::util::GtkPaperSizeUPtr>(paperSizeMenuOptions[i]);
        const PaperSize aDefaultPaperSize(currentPaperSize);
        if (paperSize->equalDimensions(aDefaultPaperSize)) {
            return i;
        }
    }
    return customPaperSizeIndex;  // Custom option is returned if no matching format is found
}
GtkWidget* PageTypeSelectionPopover::createPopover() const {
    GtkWidget* popover = gtk_popover_new();

    // Todo(cpp20): constexpr this
    std::string prefixedPageTypeActionName = G_ACTION_NAMESPACE;
    prefixedPageTypeActionName += PAGETYPE_SELECTION_ACTION_NAME;
    std::string prefixedOrientationActionName = std::string(G_ACTION_NAMESPACE) + ORIENTATION_SELECTION_ACTION_NAME;

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_popover_set_child(GTK_POPOVER(popover), GTK_WIDGET(box));

    GtkWidget* previewGrid =
            xoj::helper::PageTypeGrid::createPreviewGrid(types->getPageTypes(), prefixedPageTypeActionName);
    gtk_widget_set_margin_bottom(previewGrid, 5);
    gtk_box_append(box, previewGrid);

    gtk_box_append(box, createSeparator());

    GtkGrid* specialTypesGrid = xoj::helper::PageTypeGrid::createEmptyGrid();
    gtk_widget_set_margin_top(GTK_WIDGET(specialTypesGrid), 5);
    gtk_widget_set_margin_bottom(GTK_WIDGET(specialTypesGrid), 5);

    int gridX = 0;
    // Create a special entry for copying the current page's background
    // It has index == npos
    gtk_grid_attach(specialTypesGrid,
                    createEntryWithoutPreview(_("Copy current background"), npos, prefixedPageTypeActionName), gridX++,
                    0, 1, 1);

    // The indices of special page types start after the normal page types'
    size_t n = types->getPageTypes().size();
    for (const auto& pageInfo: types->getSpecialPageTypes()) {
        gtk_grid_attach(specialTypesGrid,
                        createEntryWithoutPreview(pageInfo->name.c_str(), n++, prefixedPageTypeActionName), gridX++, 0,
                        1, 1);
    }

    while (gridX < xoj::helper::PageTypeGrid::PAGE_TYPES_PER_ROW) {
        // Add empty cells to the grid so the buttons don't get extended
        gtk_grid_attach(specialTypesGrid, gtk_label_new(""), gridX++, 0, 1, 1);
    }

    GtkBox* pageFormatBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_margin_start(GTK_WIDGET(pageFormatBox), 10);
    gtk_widget_set_margin_end(GTK_WIDGET(pageFormatBox), 10);
    gtk_widget_set_margin_top(GTK_WIDGET(pageFormatBox), 7);
    gtk_widget_set_margin_bottom(GTK_WIDGET(pageFormatBox), 10);

    GtkBox* orientationFormatBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_add_css_class(GTK_WIDGET(orientationFormatBox), "linked");
    gtk_widget_add_css_class(GTK_WIDGET(orientationFormatBox), "orientationFormatBox");

    std::array<GtkWidget*, 2> orientationButtons{};
    orientationButtons[GTK_ORIENTATION_VERTICAL] = createOrientationButton(
            prefixedOrientationActionName, GTK_ORIENTATION_VERTICAL, "xopp-orientation-portrait");
    orientationButtons[GTK_ORIENTATION_HORIZONTAL] = createOrientationButton(
            prefixedOrientationActionName, GTK_ORIENTATION_HORIZONTAL, "xopp-orientation-landscape");

    gtk_box_append(orientationFormatBox, orientationButtons[GTK_ORIENTATION_VERTICAL]);
    gtk_box_append(orientationFormatBox, orientationButtons[GTK_ORIENTATION_HORIZONTAL]);

    gtk_box_append(pageFormatBox, GTK_WIDGET(orientationFormatBox));
    gtk_widget_set_hexpand(GTK_WIDGET(orientationFormatBox), true);

    GtkWidget* pageFormatComboBox = gtk_combo_box_new();
    PaperFormatUtils::fillPaperFormatDropDown(paperSizeMenuOptions, GTK_COMBO_BOX(pageFormatComboBox));
    gtk_widget_set_halign(pageFormatComboBox, GTK_ALIGN_END);
    gtk_combo_box_set_active(GTK_COMBO_BOX(pageFormatComboBox),
                             static_cast<gint>(getComboBoxIndexForPaperSize(selectedPageSize)));
    g_signal_connect(pageFormatComboBox, "changed", G_CALLBACK(changedPaperFormatTemplateCb),
                     const_cast<PageTypeSelectionPopover*>(this));
    gtk_box_append(pageFormatBox, pageFormatComboBox);

    gtk_box_append(box, GTK_WIDGET(specialTypesGrid));
    gtk_box_append(box, createSeparator());
    gtk_box_append(box, GTK_WIDGET(pageFormatBox));

    GtkWidget* applyToCurrentPageButton = gtk_button_new_with_label(_("Apply to current page"));
    g_signal_connect_data(
            applyToCurrentPageButton, "clicked", G_CALLBACK((+[](GtkButton*, gpointer pointerTuple) {
                auto [self, popover] =
                        *static_cast<std::tuple<const PageTypeSelectionPopover*, GtkPopover*>*>(pointerTuple);
                if (self->selectedPT) {
                    self->controller->changeCurrentPageBackground(self->selectedPT.value());
                }
                if (self->selectedPageSize && (!self->selectedPT.has_value() || !self->selectedPT->isSpecial())) {
                    self->controller->changeCurrentPageSize(self->selectedPageSize.value());
                }
            })),
            new std::tuple<const PageTypeSelectionPopover*, GtkPopover*>(this, GTK_POPOVER(popover)),
            xoj::util::closure_notify_cb<std::tuple<const PageTypeSelectionPopover*, GtkPopover*>>, GConnectFlags(0));
    gtk_widget_set_margin_start(applyToCurrentPageButton, 10);
    gtk_widget_set_margin_end(applyToCurrentPageButton, 10);
    gtk_widget_set_margin_bottom(applyToCurrentPageButton, 3);
    gtk_box_append(box, applyToCurrentPageButton);

    // We cannot "Apply to current page" if neither page type nor page size is selected...
    gtk_widget_set_sensitive(applyToCurrentPageButton,
                             this->selectedPT.has_value() || this->selectedPageSize.has_value());

    auto userdataPointerTuple = new std::tuple<GtkButton*, const PageTypeSelectionPopover*, GtkComboBox*>(
            GTK_BUTTON(applyToCurrentPageButton), this, GTK_COMBO_BOX(pageFormatComboBox));
    g_signal_connect_data(
            this->typeSelectionAction.get(), "notify::state",
            G_CALLBACK((+[](GObject* a, GParamSpec*, gpointer pointerTuple) {
                auto [btn, self, pageFormatCB] =
                        *static_cast<std::tuple<GtkButton*, const PageTypeSelectionPopover*, GtkComboBox*>*>(
                                pointerTuple);
                xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
                auto selectedIndex = getGVariantValue<size_t>(state.get());
                gtk_widget_set_sensitive(GTK_WIDGET(btn), selectedIndex != COPY_CURRENT_PLACEHOLDER ||
                                                                  self->selectedPageSize.has_value());
                // Enable page format selection for all page types except special ones (PDF and image)
                bool shouldPageFormatSettingsBeEnabled =
                        selectedIndex == COPY_CURRENT_PLACEHOLDER || selectedIndex < self->types->getPageTypes().size();
                gtk_widget_set_sensitive(GTK_WIDGET(pageFormatCB), shouldPageFormatSettingsBeEnabled);

                g_simple_action_set_enabled(self->orientationAction.get(),
                                            shouldPageFormatSettingsBeEnabled && self->selectedPageSize.has_value());
            })),
            userdataPointerTuple,
            xoj::util::closure_notify_cb<std::tuple<GtkButton*, const PageTypeSelectionPopover*, GtkComboBox*>>,
            GConnectFlags(0));

    g_signal_connect_data(
            this->pageSizeChangedAction.get(), "activate",
            G_CALLBACK((+[](GSimpleAction*, GVariant*, gpointer pointerTuple) {
                auto [btn, self] = *static_cast<std::tuple<GtkWidget*, const PageTypeSelectionPopover*>*>(pointerTuple);
                gtk_widget_set_sensitive(btn, self->selectedPT.has_value() || self->selectedPageSize.has_value());
            })),
            new std::tuple<GtkWidget*, const PageTypeSelectionPopover*>(applyToCurrentPageButton, this),
            xoj::util::closure_notify_cb<std::tuple<GtkWidget*, const PageTypeSelectionPopover*>>, GConnectFlags(0));

    g_signal_connect_object(this->comboBoxChangeSelectionAction.get(), "activate",
                            G_CALLBACK(+[](GSimpleAction* action, GVariant* comboBoxIndex, gpointer comboBoxPtr) {
                                auto* comboBox = static_cast<GtkWidget*>(comboBoxPtr);
                                gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox),
                                                         static_cast<gint>(getGVariantValue<guint32>(comboBoxIndex)));
                            }),
                            pageFormatComboBox, GConnectFlags(0));

    GtkWidget* applyToAllPagesButton = gtk_button_new_with_label(_("Apply to all pages"));
    g_signal_connect_data(
            applyToAllPagesButton, "clicked", G_CALLBACK((+[](GtkWidget*, gpointer pointerTuple) {
                auto [self, popover] =
                        *static_cast<std::tuple<const PageTypeSelectionPopover*, GtkPopover*>*>(pointerTuple);
                if (self->selectedPT) {
                    self->controller->applyBackgroundToAllPages(self->selectedPT.value());
                } else {
                    self->controller->applyCurrentPageBackgroundToAll();
                }

                if (self->selectedPageSize) {
                    self->controller->applyPageSizeToAllPages(self->selectedPageSize.value());
                } else {
                    const PageRef page = self->control->getCurrentPage();
                    self->controller->applyPageSizeToAllPages(PaperSize(page->getWidth(), page->getHeight()));
                }
            })),
            new std::tuple<const PageTypeSelectionPopover*, GtkPopover*>(this, GTK_POPOVER(popover)),
            xoj::util::closure_notify_cb<std::tuple<const PageTypeSelectionPopover*, GtkPopover*>>, GConnectFlags(0));
    gtk_widget_set_margin_start(applyToAllPagesButton, 10);
    gtk_widget_set_margin_end(applyToAllPagesButton, 10);
    gtk_widget_set_margin_top(applyToAllPagesButton, 3);
    gtk_widget_set_margin_bottom(applyToAllPagesButton, 10);
    gtk_box_append(box, applyToAllPagesButton);

    return popover;
}
void PageTypeSelectionPopover::changedPaperFormatTemplateCb(GtkComboBox* widget, PageTypeSelectionPopover* self) {
    if (self->ignoreComboBoxSelectionChange) {
        return;
    }
    auto selected = static_cast<uint32_t>(gtk_combo_box_get_active(widget));
    if (selected < self->customPaperSizeIndex) {
        auto orientation = static_cast<PaperOrientation>(getGVariantValue<bool>(
                xoj::util::GVariantSPtr(g_action_get_state(G_ACTION(self->orientationAction.get())), xoj::util::adopt)
                        .get()));

        PaperSize paperSize(std::get<xoj::util::GtkPaperSizeUPtr>(self->paperSizeMenuOptions[selected]));
        if (paperSize.orientation() != orientation) {
            paperSize.swapWidthHeight();
        }

        self->setSelectedPaperSize<false>(paperSize);
    } else if (selected == self->customPaperSizeIndex) {
        std::unique_ptr<xoj::popup::PopupWindowWrapper<xoj::popup::FormatDialog>> popup;
        auto callback = [self](double width, double height) {
            self->setSelectedPaperSize<true>(PaperSize(width, height));
        };
        if (self->selectedPageSize) {
            popup = std::make_unique<xoj::popup::PopupWindowWrapper<xoj::popup::FormatDialog>>(
                    self->control->getGladeSearchPath(), self->settings, self->selectedPageSize->width,
                    self->selectedPageSize->height, callback);
        } else {
            PageTemplateSettings model;
            model.parse(self->settings->getPageTemplate());
            popup = std::make_unique<xoj::popup::PopupWindowWrapper<xoj::popup::FormatDialog>>(
                    self->control->getGladeSearchPath(), self->settings, model.getPageWidth(), model.getPageHeight(),
                    callback);
        }
        popup->show(self->control->getGtkWindow());

    } else if (selected == self->copyCurrentPaperSizeIndex) {
        self->setSelectedPaperSize<false>(std::nullopt);
    }
}
void PageTypeSelectionPopover::changedOrientationSelectionCallback(GSimpleAction* ga, GVariant* parameter,
                                                                   PageTypeSelectionPopover* self) {
    g_simple_action_set_state(ga, parameter);
    self->selectedOrientation = static_cast<PaperOrientation>(getGVariantValue<bool>(parameter));
    if (self->selectedPageSize && (self->selectedPageSize->orientation() != self->selectedOrientation)) {
        self->selectedPageSize->swapWidthHeight();
        self->controller->setPaperSizeForNewPages(self->selectedPageSize);
    }
}

void PageTypeSelectionPopover::entrySelected(const PageTypeInfo*) {
    this->controller->setPageTypeForNewPages(this->selectedPT);
}
