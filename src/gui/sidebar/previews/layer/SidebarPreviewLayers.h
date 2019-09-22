/*
 * Xournal++
 *
 * Previews of the layers in the current page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/layer/LayerCtrlListener.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"

#include <XournalType.h>

class SidebarPreviewLayers : public SidebarPreviewBase, public LayerCtrlListener
{
public:
	SidebarPreviewLayers(Control* control, GladeGui* gui, SidebarToolbar* toolbar);
	virtual ~SidebarPreviewLayers();

public:
	virtual void rebuildLayerMenu();
	virtual void layerVisibilityChanged();

public:
	/**
	 * Called when an action is performed
	 */
	void actionPerformed(SidebarActions action);

	void enableSidebar();

	/**
	 * @overwrite
	 */
	virtual string getName();

	/**
	 * @overwrite
	 */
	virtual string getIconName();

	/**
	 * Update the preview images
	 * @overwrite
	 */
	virtual void updatePreviews();

	/**
	 * Select a layer
	 */
	void layerSelected(size_t layerIndex);

	/**
	 * A layer was hidden / showed
	 */
	void layerVisibilityChanged(int layerIndex, bool enabled);

protected:
	void updateSelectedLayer();

public:
	// DocumentListener interface (only the part which is not handled by SidebarPreviewBase)
	virtual void pageSizeChanged(size_t page);
	virtual void pageChanged(size_t page);

private:
	/**
	 * Layer Controller
	 */
	LayerController* lc;
};
