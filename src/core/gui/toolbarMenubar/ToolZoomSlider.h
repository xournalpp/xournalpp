/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkRange, GtkWidget

#include "control/zoom/ZoomListener.h"  // for ZoomListener
#include "gui/IconNameHelper.h"         // for IconNameHelper

#include "AbstractSliderItem.h"  // for NewAbstractSliderItem

class ZoomControl;
class ActionDatabase;

class ToolZoomSlider: public AbstractSliderItem {
public:
    ToolZoomSlider(std::string id, ZoomControl* zoom, IconNameHelper iconNameHelper, ActionDatabase& db);
    ~ToolZoomSlider() override = default;

    xoj::util::WidgetSPtr createItem(bool horizontal) override;

    /**
     * @brief Function to convert from the zoom scale to the slider's position. (e.g. for log scaling)
     */
    static double scaleFunction(double x);

    /**
     * @brief Function to convert from the slider's position to the zoom scale. (e.g. for log scaling)
     */
    static double scaleInverseFunction(double x);

    static std::string formatSliderValue(double value);

protected:
    static constexpr bool DISPLAY_VALUE = true;

    std::string getToolDisplayName() const override;

    GtkWidget* getNewToolIcon() const override;

protected:
    std::string iconName;
    ZoomControl* zoomCtrl;

    template <class FinalSliderType>
    friend class SliderItemCreationHelper;
};
