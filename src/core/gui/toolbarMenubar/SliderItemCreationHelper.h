/*
 * Xournal++
 *
 * Abstract representation of slider-based tools.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gtk/gtk.h>  // for GtkRange

#include "control/actions/ActionRef.h"
#include "util/GVariantTemplate.h"  // for makeGVariant
#include "util/GtkUtil.h"           // for setWidgetFollowActionEnabled
#include "util/glib_casts.h"        // for wrap_for_g_callback_v
#include "util/gtk4_helper.h"       // for gtk_scale_set_format_value_func

#include "AbstractSliderItem.h"

/**
 * @brief Helper function. We rely on a template to avoid adding a vtable to every callback data
 */
template <class FinalSliderType>
class SliderItemCreationHelper {
public:
    static void valueChangedCb(GtkRange* slider, gpointer gAction) {
        double state = FinalSliderType::scaleInverseFunction(gtk_range_get_value(slider));
        g_action_change_state(G_ACTION(gAction), makeGVariant(state));
    }

    static xoj::util::WidgetSPtr createItem(AbstractSliderItem* self, bool horizontal) {
        GtkOrientation orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
        const double min = FinalSliderType::scaleFunction(self->range.min);
        const double max = FinalSliderType::scaleFunction(self->range.max);
        const double fineStepSize = (max - min) / self->range.nbFineSteps;
        const double coarseStepSize = (max - min) / self->range.nbCoarseSteps;

        GtkRange* slider = GTK_RANGE(gtk_scale_new_with_range(orientation, min, max, fineStepSize));
        gtk_range_set_increments(slider, fineStepSize, coarseStepSize);

        if (horizontal) {
            gtk_widget_set_size_request(GTK_WIDGET(slider), 120, 16);
        } else {
            gtk_widget_set_size_request(GTK_WIDGET(slider), 16, 120);
        }

        // gAction does not own the return GVariant and it is not floating either!
        xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(self->gAction.get())), xoj::util::adopt);
        gtk_range_set_value(slider, FinalSliderType::scaleFunction(getGVariantValue<double>(state.get())));

        g_signal_connect_object(slider, "value-changed", G_CALLBACK(valueChangedCb), self->gAction.get(),
                                GConnectFlags(0));

        g_signal_connect_object(
                self->gAction.get(), "notify::state", G_CALLBACK(+[](GObject* action, GParamSpec*, gpointer slider) {
                    // action does not own the return GVariant and it is not floating either!
                    xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(action)), xoj::util::adopt);
                    xoj_assert(state);
                    g_signal_handlers_block_by_func(slider, (gpointer)valueChangedCb, action);
                    gtk_range_set_value(GTK_RANGE(slider),
                                        FinalSliderType::scaleFunction(getGVariantValue<double>(state.get())));
                    g_signal_handlers_unblock_by_func(slider, (gpointer)valueChangedCb, action);
                }),
                slider, GConnectFlags(0));

        xoj::util::gtk::setWidgetFollowActionEnabled(GTK_WIDGET(slider), G_ACTION(self->gAction.get()));

        if constexpr (FinalSliderType::DISPLAY_VALUE) {
            gtk_scale_set_draw_value(GTK_SCALE(slider), true);
            gtk_scale_set_format_value_func(
                    GTK_SCALE(slider),
                    +[](GtkScale*, double value, gpointer) -> char* {
                        return g_strdup(FinalSliderType::formatSliderValue(value).c_str());
                    },
                    nullptr, nullptr);
        }

        return xoj::util::WidgetSPtr(GTK_WIDGET(slider), xoj::util::adopt);
    }
};
