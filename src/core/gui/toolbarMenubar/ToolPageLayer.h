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

#include <map>

#include "control/layer/LayerCtrlListener.h"
#include "gui/IconNameHelper.h"
#include "model/Layer.h"

#include "AbstractToolItem.h"

class GladeGui;
class PopupMenuButton;
class LayerController;

class ToolPageLayer: public AbstractToolItem, public LayerCtrlListener {
public:
    ToolPageLayer(LayerController* lc, GladeGui* gui, ActionHandler* handler, std::string id, ActionType type,
                  IconNameHelper iconNameHelper);
    ~ToolPageLayer() override;

public:
    std::string getToolDisplayName() const override;

    // LayerCtrlListener
public:
    void rebuildLayerMenu() override;
    void layerVisibilityChanged() override;

protected:
    GtkWidget* createSpecialMenuEntry(const std::string& name);
    void createSeparator();

    /**
     * Add special button to the top of the menu
     */
    void addSpecialButtonTop();

    /**
     * Rebuild the Menu
     */
    void updateMenu();

    /**
     * Update selected layer, update visible layer
     */
    void updateLayerData();

    GtkToolItem* newItem() override;
    GtkWidget* getNewToolIcon() const override;
    GdkPixbuf* getNewToolPixbuf() const override;

private:
    void createLayerMenuItem(const std::string& text, Layer::Index layerId);
    void layerMenuClicked(GtkWidget* menu);
    void createLayerMenuItemShow(Layer::Index layerId);
    void layerMenuShowClicked(GtkWidget* menu);

    void selectLayer(Layer::Index layerId);

private:
    LayerController* lc = nullptr;
    GladeGui* gui = nullptr;

    GtkWidget* layerLabel = nullptr;
    GtkWidget* layerButton = nullptr;
    GtkWidget* menu = nullptr;

    // Widget for the layer menu. Index = 0 is for background.
    std::vector<GtkWidget*> layerItems;
    std::vector<GtkWidget*> showLayerItems;

    PopupMenuButton* popupMenuButton = nullptr;
    unsigned int menuY = 0;

    /**
     * Menu is currently updating - ignore events
     */
    bool inMenuUpdate = false;
    IconNameHelper iconNameHelper;
};
