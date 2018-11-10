/*
 * Xournal++
 *
 * Image view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseElementView.h"

#include <XournalType.h>

class ImageElementView : public BaseElementView
{
public:
	ImageElementView();
	~ImageElementView();

private:
	XOJ_TYPE_ATTRIB;
};
