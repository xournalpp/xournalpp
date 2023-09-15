#include "PageTypeSelectionPopoverGridOnly.h"

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <glib.h>
#include <gtk/gtkactionable.h>

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "gui/CreatePreviewImage.h"
#include "gui/dialog/PageTemplateDialog.h"
#include "model/PageType.h"  // for PageType
#include "util/Assert.h"
#include "util/raii/GVariantSPtr.h"

namespace {
static constexpr auto PAGE_TYPES_PER_ROW = 4;
static constexpr auto G_ACTION_NAMESPACE = "popover";
static constexpr auto SELECTION_ACTION_NAME = "select-page-type-template";

GtkWidget* createPreviewGrid(const std::vector<std::unique_ptr<PageTypeInfo>>& pageTypes,
                             const std::string_view& prefixedActionName) {
    /*
     * Todo(gtk4): replace the GtkMenu with a GtkPopover, using PageTypeSelectionPopover::createPreviewGrid instead.
     * Note that GtkPopover is no good with GTK 3, as the popover gets cropped to the dialog window (so part of it is
     * not displayed). GtkMenus don't have this restriction but are removed in GTK 4. Popovers in GTK 4 do not have this
     * restriction.
     */
    GtkMenu* gtkMenu = GTK_MENU(gtk_menu_new());

    size_t n = 0;
    unsigned int gridX = 0;
    unsigned int gridY = 0;

    for (const auto& pageInfo: pageTypes) {
        // Special page types do not get a preview
        xoj_assert(!pageInfo->page.isSpecial());
        auto* entry = gtk_check_menu_item_new();
        gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(entry), true);
        gtk_actionable_set_action_name(GTK_ACTIONABLE(entry), prefixedActionName.data());
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(entry), xoj::util::makeGVariantSPtr(n++).get());
        GtkWidget* preview = xoj::helper::createPreviewImage(pageInfo->page);
        gtk_widget_set_tooltip_text(preview, pageInfo->name.c_str());
        gtk_container_add(GTK_CONTAINER(entry), preview);  // takes ownership of preview
        if (gridX >= PAGE_TYPES_PER_ROW) {
            gridX = 0;
            gridY++;
        }
        gtk_menu_attach(gtkMenu, entry, gridX, gridX + 1, gridY, gridY + 1);
        gridX++;
    }
    return GTK_WIDGET(gtkMenu);
}
};  // namespace


PageTypeSelectionPopoverGridOnly::PageTypeSelectionPopoverGridOnly(PageTypeHandler* typesHandler,
                                                                   const Settings* settings,
                                                                   xoj::popup::PageTemplateDialog* parent):
        PageTypeSelectionMenuBase(typesHandler, settings, SELECTION_ACTION_NAME),
        parent(parent),
        popover(createPopover()) {

    xoj::util::GObjectSPtr<GSimpleActionGroup> group(g_simple_action_group_new(), xoj::util::adopt);
    g_action_map_add_action(G_ACTION_MAP(group.get()), G_ACTION(typeSelectionAction.get()));
    gtk_widget_insert_action_group(GTK_WIDGET(parent->getWindow()), G_ACTION_NAMESPACE, G_ACTION_GROUP(group.get()));
}

xoj::util::WidgetSPtr PageTypeSelectionPopoverGridOnly::createPopover() {
    // Todo(cpp20): constexpr this
    std::string prefixedActionName = G_ACTION_NAMESPACE;
    prefixedActionName += ".";
    prefixedActionName += SELECTION_ACTION_NAME;

    xoj::util::WidgetSPtr popover(createPreviewGrid(types->getPageTypes(), prefixedActionName), xoj::util::adopt);
    gtk_widget_show_all(popover.get());
    return popover;
}

void PageTypeSelectionPopoverGridOnly::entrySelected(const PageTypeInfo* info) {
    xoj_assert(info);
    parent->changeCurrentPageBackground(info);
}
