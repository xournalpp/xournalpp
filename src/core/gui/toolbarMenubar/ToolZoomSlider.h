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

#include "AbstractSliderItem.h"  // for AbstractSliderItem

class ZoomControl;
class ActionDatabase;

class ToolZoomSlider: public AbstractSliderItem, public ZoomListener {
public:
    ToolZoomSlider(std::string id, ZoomControl* zoom, IconNameHelper iconNameHelper, ActionDatabase& db);
    ~ToolZoomSlider() override;

protected:
    void onSliderChanged(double value) override;
    void onSliderButtonPress() override;
    void onSliderButtonRelease() override;
    void onSliderHoverScroll() override;
    std::string formatSliderValue(double value) const override;
    void configure(GtkRange* slider, bool isHorizontal) const override;

    void zoomChanged() override;
    void zoomRangeValuesChanged() override;

    std::string getToolDisplayName() const override;

protected:
    GtkWidget* getNewToolIcon() const override;

    double scaleFunc(double x) const override;
    double scaleFuncInv(double x) const override;

private:
    class Impl;

    std::unique_ptr<Impl> pImpl;
};
