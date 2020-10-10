/*
 * Xournal++
 *
 * Handler for layer selection and hiding / showing layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "control/Actions.h"
#include "gui/dialog/RenameLayerDialog.h"
#include "model/DocumentListener.h"
#include "model/PageRef.h"

#include "XournalType.h"

class LayerCtrlListener;
class Control;
class Layer;

class LayerController: public DocumentListener {
public:
    LayerController(Control* control);
    ~LayerController() override = default;

public:
    void documentChanged(DocumentChangeType type) override;
    void pageSelected(size_t page) override;

public:
    void insertLayer(PageRef page, Layer* layer, int layerPos);
    void removeLayer(PageRef page, Layer* layer);
    void addLayer(PageRef page, Layer* layer);

    // Listener handling
public:
    void addListener(LayerCtrlListener* listener);
    void removeListener(LayerCtrlListener* listener);

protected:
    void fireRebuildLayerMenu();
    void fireLayerVisibilityChanged();

public:
    bool actionPerformed(ActionType type);

    /**
     * Show all layer on the current page
     */
    void showAllLayer();

    /**
     * Hide all layer on the current page
     */
    void hideAllLayer();

    /**
     * Show / Hide all layer on the current page
     */
    void hideOrHideAllLayer(bool show);

    void addNewLayer();
    void deleteCurrentLayer();
    void copyCurrentLayer();
    void moveCurrentLayer(bool up);
    void switchToLay(int layer, bool hideShow = false);
    void setLayerVisible(int layerId, bool visible);

    PageRef getCurrentPage();
    size_t getCurrentPageId() const;

    /**
     * @return Layer count of the current page
     */
    size_t getLayerCount();

    /**
     * @return Current layer ID
     */
    size_t getCurrentLayerId();

    /**
     * @return Current layer name
     */
    std::string getCurrentLayerName();

    /**
     * @return Get layer name by layer id
     */
    std::string getLayerNameById(int id);

    /**
     * Sets current layer name
     */
    void setCurrentLayerName(const std::string& newName);

    /**
     * Make sure there is at least one layer on the page
     */
    void ensureLayerExists(PageRef page);

private:
    Control* control;

    std::list<LayerCtrlListener*> listener;

    size_t selectedPage;
};
