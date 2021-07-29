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
#include "util/IconNameHelper.h"

#include "AbstractToolItem.h"

class GladeGui;
class PopupMenuButton;
class LayerController;

class ToolPageLayer: public AbstractToolItem, public LayerCtrlListener {
public:
    ToolPageLayer(LayerController* lc, GladeGui* gui, ActionHandler* handler, std::string id, ActionType type,
                  IconNameHelper iconNameHelper);
    virtual ~ToolPageLayer();

public:
    virtual std::string getToolDisplayName();

    // LayerCtrlListener
public:
    virtual void rebuildLayerMenu();
    virtual void layerVisibilityChanged();

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

    virtual GtkToolItem* newItem();
    virtual GtkWidget* getNewToolIcon();

private:
    void createLayerMenuItem(const std::string& text, int layerId);
    void layerMenuClicked(GtkWidget* menu);
    void createLayerMenuItemShow(int layerId);
    void layerMenuShowClicked(GtkWidget* menu);

    void selectLayer(int layerId);

private:
    LayerController* lc = nullptr;
    GladeGui* gui = nullptr;

    GtkWidget* layerLabel = nullptr;
    GtkWidget* layerButton = nullptr;
    GtkWidget* menu = nullptr;

    std::map<int, GtkWidget*> layerItems;
    std::map<int, GtkWidget*> showLayerItems;

    PopupMenuButton* popupMenuButton = nullptr;
    int menuY = 0;

    /**
     * Menu is currently updating - ignore events
     */
    bool inMenuUpdate = false;
    IconNameHelper iconNameHelper;
};
