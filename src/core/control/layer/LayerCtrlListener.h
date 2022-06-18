/*
 * Xournal++
 *
 * Layer Controller listener
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class LayerController;

class LayerCtrlListener {
public:
    LayerCtrlListener();
    virtual ~LayerCtrlListener();

public:
    void registerListener(LayerController* handler);
    void unregisterListener();

    virtual void rebuildLayerMenu();
    virtual void layerVisibilityChanged();

private:
    LayerController* handler;
};
