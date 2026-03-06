#include "PageTypeSelectionPopoverGridOnly.h"

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <glib.h>
#include <gtk/gtk.h>

#include "control/pagetype/PageTypeHandler.h"
#include "gui/dialog/PageTemplateDialog.h"
#include "util/Assert.h"

#include "PageTypeSelectionPopoverGridHelper.h"

static constexpr auto G_ACTION_NAMESPACE = "popover";
static constexpr auto SELECTION_ACTION_NAME = "select-page-type-template";


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

    xoj::util::WidgetSPtr popover(gtk_popover_new(), xoj::util::adopt);
    gtk_popover_set_child(GTK_POPOVER(popover.get()),
                          xoj::helper::PageTypeGrid::createPreviewGrid(types->getPageTypes(), prefixedActionName));
    return popover;
}

void PageTypeSelectionPopoverGridOnly::entrySelected(const PageTypeInfo* info) {
    xoj_assert(info);
    parent->changeCurrentPageBackground(info);
}
