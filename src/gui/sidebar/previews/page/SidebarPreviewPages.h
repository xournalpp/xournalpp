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
	virtual const char* getName();

	/**
	 * @overwrite
	 */
	virtual const char* getIconName();

private:
	XOJ_TYPE_ATTRIB;
};
