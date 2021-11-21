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

#include <memory>
#include <string>

#include "AbstractSliderItem.h"

class Control;
class ActionHandler;
class IconNameHelper;

class ToolWindowOpacitySlider: public AbstractSliderItem {
public:
    ToolWindowOpacitySlider(std::string id, ActionHandler* handler, ActionType type, Control* control,
                            IconNameHelper iconNameHelper);
    virtual ~ToolWindowOpacitySlider();

    virtual std::string getToolDisplayName() override;

protected:
    virtual void configure(GtkRange* slider, bool isHorizontal) const override;

    virtual GtkWidget* getNewToolIcon() override;
    virtual void onSliderChanged(double value) override;

    double scaleFunc(double x) const override;
    double scaleFuncInv(double x) const override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
