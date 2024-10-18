/*
 * Xournal++
 *
 * Handles page selection menu
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget

#include "control/Control.h"
#include "control/settings/PageTemplateSettings.h"
#include "gui/PaperFormatUtils.h"
#include "gui/PopoverFactory.h"
#include "gui/menus/PageTypeSelectionMenuBase.h"
#include "model/PaperSize.h"
#include "util/raii/GObjectSPtr.h"

class PageBackgroundChangeController;
class PageTypeHandler;
class PageTypeInfo;
class Settings;

class PageTypeSelectionPopover final: public PageTypeSelectionMenuBase, public PopoverFactory {
public:
    PageTypeSelectionPopover(Control* control, Settings* settings, GtkApplicationWindow* win);
    ~PageTypeSelectionPopover() override = default;

public:
    GtkWidget* createPopover() const override;

    /**
     * @brief Sets the selected paper size of the menu.
     * @tparam changeComboBoxSelection Whether the combo box selection will be changed to a fitting option
     */
    template <bool changeComboBoxSelection = true>
    void setSelectedPaperSize(const std::optional<PaperSize>& newPageSize);

private:
    void entrySelected(const PageTypeInfo* info) override;

    static void changedOrientationSelectionCallback(GSimpleAction* ga, GVariant* parameter,
                                                    PageTypeSelectionPopover* self);
    static void changedPaperFormatTemplateCb(GtkComboBox* widget, PageTypeSelectionPopover* dlg);

    [[nodiscard]] unsigned int getComboBoxIndexForPaperSize(const std::optional<PaperSize>& paperSize) const;

private:
    Control* control;
    PageBackgroundChangeController* controller;
    Settings* settings;

    std::optional<PaperSize> selectedPageSize;

    PaperOrientation selectedOrientation;
    xoj::util::GObjectSPtr<GSimpleAction> orientationAction;

    // By activating the comboBoxChangeSelectionAction the option selected by the page size comboBox is changed to the
    // specified index
    xoj::util::GObjectSPtr<GSimpleAction> comboBoxChangeSelectionAction;

    // The pageSizeChangedAction is activated when the pageSize changed
    xoj::util::GObjectSPtr<GSimpleAction> pageSizeChangedAction;

    PaperFormatUtils::PaperFormatMenuOptionVector paperSizeMenuOptions;

    bool ignoreComboBoxSelectionChange = false;

    uint32_t customPaperSizeIndex;  // this - 1 is the last index of an actual PaperSize option
    uint32_t copyCurrentPaperSizeIndex;
};
