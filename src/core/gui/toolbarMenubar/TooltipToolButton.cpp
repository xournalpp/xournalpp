#include "TooltipToolButton.h"

#include <utility>  // for move

#include <glib.h>
#include <gtk/gtk.h>

#include "util/glib_casts.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

TooltipToolButton::TooltipToolButton(std::string id, Category cat, Action action, std::string iconName,
                                     std::string description, std::function<std::string()> fetchTooltip):
        ToolButton(std::move(id), cat, action, std::move(iconName), std::move(description), false),
        fetchTooltip(std::move(fetchTooltip)) {}

auto TooltipToolButton::createItem(ToolbarSide side) -> Widgetry {
    auto widgetry = ToolButton::createItem(side);

    gtk_widget_set_has_tooltip(widgetry.item.get(), true);

    auto* cloneFetchTooltip = new std::function<std::string()>(this->fetchTooltip);

    g_signal_connect_data(widgetry.item.get(), "query-tooltip",
                          G_CALLBACK(+[](GtkWidget*, gint, gint, gboolean, GtkTooltip* tooltip, gpointer d) {
                              auto fn = static_cast<std::function<std::string()>*>(d);
                              gtk_tooltip_set_text(tooltip, (*fn)().c_str());
                              return true;
                          }),
                          cloneFetchTooltip, xoj::util::closure_notify_cb<std::function<std::string()>>,
                          GConnectFlags(0));

    return widgetry;
}
