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

#include <gtk/gtk.h>  // for GtkToolItem, GtkRange

#include "control/actions/ActionRef.h"

#include "ItemWithNamedIcon.h"  // for ItemWithNamedIcon

class AbstractSliderItem: public ItemWithNamedIcon {
public:
    struct SliderRange {
        // Internal units: minimum and maximum slider values.
        double min;
        double max;

        // Number of fine/coarse steps between min and max.
        int nbFineSteps;
        int nbCoarseSteps;
    };

public:
    /**
     * @param id Unique identifier for this. e.g. `TOOL_FOO_SLIDER`.
     * @param range The minimum/maximum/step in internal units. {@see #scaleFunc}.
     * @param gAction A GAction whose (double) state is synced with the slider's value
     */
    AbstractSliderItem(std::string id, Category cat, SliderRange range, ActionRef gAction, std::string iconName);
    ~AbstractSliderItem() override = default;

protected:
    const char* getIconName() const override;

protected:
    SliderRange range;
    ActionRef gAction;
    std::string iconName;

    template <class FinalSliderType>
    friend class SliderItemCreationHelper;
};
