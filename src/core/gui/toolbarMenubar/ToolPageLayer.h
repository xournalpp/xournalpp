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

#include "AbstractToolItem.h"  // for AbstractToolItem

class LayerController;
class IconNameHelper;

class ToolPageLayer: public AbstractToolItem, public LayerCtrlListener {
public:
    ToolPageLayer(std::string id, LayerController* lc, IconNameHelper iconNameHelper);
    ~ToolPageLayer() override;

    class ShowLayerEntry;

public:
    std::string getToolDisplayName() const override;
    GtkWidget* createItem(bool horizontal) override;
    void setUsed(bool used) override;  ///< Cleans up the layer entries

    // LayerCtrlListener
    void rebuildLayerMenu() override;
    void layerVisibilityChanged() override;
    void updateSelectedLayer() override;

protected:
    /**
     * @brief Build the widgetry `this->grid` add adds it to `this->box`. Also updates this->layerLabel.
     * WARNING: Remove any previously added grid from box before calling this
     */
    void setupLayersGrid();

    GtkWidget* getNewToolIcon() const override;

private:
    LayerController* lc = nullptr;

    xoj::util::WidgetSPtr layerLabel;            ///< Active layer name displayed in the toolbar
    xoj::util::WidgetSPtr popover;               ///< The popover
    xoj::util::WidgetSPtr box;                   ///< Main box of the popover
    xoj::util::WidgetSPtr grid;                  ///< Last child of the box, containing the layer entries
    std::vector<ShowLayerEntry> showLayerItems;  ///< Widgets for the layer menu. Index = 0 is for background.
    std::string iconName;
};
