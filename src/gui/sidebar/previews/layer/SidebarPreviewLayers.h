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
	virtual const char* getName();

	/**
	 * @overwrite
	 */
	virtual const char* getIconName();

private:
	XOJ_TYPE_ATTRIB;
};
