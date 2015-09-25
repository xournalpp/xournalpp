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

#include "gui/sidebar/previews/base/SidebarPreviewBase.h"

#include <XournalType.h>

class SidebarPreviewLayers : public SidebarPreviewBase
{
public:
	SidebarPreviewLayers(Control* control);
	virtual ~SidebarPreviewLayers();

public:

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

public:
	// DocumentListener interface (only the part which is not handled by SidebarPreviewBase)
	virtual void pageSizeChanged(int page);
	virtual void pageChanged(int page);
	virtual void pageSelected(int page);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The layers of this page are displayed
	 */
	int displayedPage;
};
