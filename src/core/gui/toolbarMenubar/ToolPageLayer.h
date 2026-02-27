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

#include <string>  // for string
#include <vector>  // for vector

#include <gtk/gtk.h>                // for GtkWidget, GtkToolItem

#include "control/layer/LayerCtrlListener.h"  // for LayerCtrlListener
#include "model/Layer.h"                      // for Layer, Layer::Index
#include "util/raii/GObjectSPtr.h"            // for WidgetSPtr

#include "ItemWithNamedIcon.h"  // for ItemWithNamedIcon

class LayerController;
class IconNameHelper;

class ToolPageLayer: public ItemWithNamedIcon {
public:
    ToolPageLayer(std::string id, LayerController* lc, IconNameHelper iconNameHelper);
    ~ToolPageLayer() override;

public:
    std::string getToolDisplayName() const override;
    Widgetry createItem(ToolbarSide side) override;

    const char* getIconName() const override;

private:
    LayerController* lc = nullptr;
    std::string iconName;
};
