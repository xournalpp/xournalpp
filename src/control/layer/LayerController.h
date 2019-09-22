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

#include <XournalType.h>

#include "control/Actions.h"
#include "model/DocumentListener.h"
#include "model/PageRef.h"

class LayerCtrlListener;
class Control;
class Layer;

class LayerController : public DocumentListener
{
public:
	LayerController(Control* control);
	virtual ~LayerController();

public:
	void documentChanged(DocumentChangeType type);
	void pageSelected(size_t page);

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
	size_t getCurrentPageId();

	/**
	 * @return Layer count of the current page
	 */
	size_t getLayerCount();

	/**
	 * @return Current layer ID
	 */
	size_t getCurrentLayerId();

	/**
	 * Make sure there is at least one layer on the page
	 */
	void ensureLayerExists(PageRef page);

private:
	Control* control;

	std::list<LayerCtrlListener*> listener;

	size_t selectedPage;
};
