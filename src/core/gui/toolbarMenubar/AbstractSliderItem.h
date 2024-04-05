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

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <gtk/gtk.h>  // for GtkToolItem, GtkRange

#include "control/actions/ActionRef.h"

#include "AbstractToolItem.h"  // for AbstractToolItem

class AbstractSliderItem: public AbstractToolItem {
public:
    struct SliderRange {
        // Internal units: minimum and maximum slider values.
        double min;
        double max;

        // Number of fine/coarse steps between min and max.
        double fineSteps;
        double coarseSteps;
    };

public:
    /**
     * @param id Unique identifier for this. e.g. `TOOL_FOO_SLIDER`.
     * @param range The minimum/maximum/step in internal units. {@see #scaleFunc}.
     * @param enablingGAction If provided, the slider will be disabled whenever the GAction is.
     */
    AbstractSliderItem(std::string id, SliderRange range, ActionRef enablingGAction);
    ~AbstractSliderItem() override;

    xoj::util::WidgetSPtr createItem(bool horizontal) override;

protected:
    virtual void onSliderButtonPress();
    virtual void onSliderButtonRelease();
    virtual void onSliderHoverScroll();
    virtual void onSliderChanged(double value) = 0;

    /**
     * Convert a given (internal) value to a user-readable representation.
     * @param value The value of the slider (GTK slider value as processed by scaleFunc).
     */
    virtual std::string formatSliderValue(double value) const;

    /**
     * Called just after the slider has been created and set up.
     *
     * If overridden, the super method (AbstractSliderItem::configure)
     * should still be called.
     *
     * Provided to allow resizing and otherwise modifying
     * @param slider the just-constructed slider.
     * @param isHorizontal `true` iff the given slider is horizontal.
     *
     * Note that the given slider may not be associated with this
     * when given to `configure`. In other words, don't assume
     * `slider == getSliderWidget()`!
     */
    virtual void configure(GtkRange* slider, bool isHorizontal) const;

protected:
    /**
     * @return nullptr if no slider widget has been created for this,
     *         otherwise, returns a pointer to the slider for this.
     */
    GtkRange* getSliderWidget();

    /**
     * @return whether this' slider is horizontal. Returns false if no such slider exists.
     */
    bool isCurrentHorizontal() const;

    /**
     * @param x Actual value of the slider.
     * @return Internal value.
     *
     * Maps GTK internal values to slider values.
     */
    virtual double scaleFunc(double x) const = 0;

    /**
     * @param x Internal value.
     * @return Corresponding internal value.
     */
    virtual double scaleFuncInv(double x) const = 0;

private:
    class Impl;

    std::unique_ptr<Impl> pImpl;
};
