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

    Widgetry createItem(ToolbarSide side) override;

protected:
    static constexpr bool DISPLAY_VALUE = true;
    static std::string formatSliderValue(double value);

    std::string getToolDisplayName() const override;

protected:
    GtkWidget* getNewToolIcon() const override;

protected:
    /**
     * @brief Function to convert from the GAction's state value to the slider's position. (e.g. for log scaling)
     */
    static double scaleFunction(double x);

    /**
     * @brief Function to convert from the slider's position to the GAction's state value. (e.g. for log scaling)
     */
    static double scaleInverseFunction(double x);

protected:
    std::string iconName;
    ZoomControl* zoomCtrl;

    template <class FinalSliderType>
    friend class SliderItemCreationHelper;
};
