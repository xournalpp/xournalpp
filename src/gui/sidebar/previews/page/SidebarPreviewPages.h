/*
 * Xournal++
 *
 * Previews of the pages in the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/sidebar/previews/base/SidebarPreviewBase.h"

#include <XournalType.h>

class SidebarPreviewPages : public SidebarPreviewBase
{
public:
	SidebarPreviewPages(Control* control);
	virtual ~SidebarPreviewPages();

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
	virtual void pageInserted(int page);
	virtual void pageDeleted(int page);

private:
	XOJ_TYPE_ATTRIB;

};
