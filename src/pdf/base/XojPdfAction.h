/*
 * Xournal++
 *
 * PDF Action Abstraction Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/LinkDestination.h"

#include <XournalType.h>

class XojPdfAction
{
public:
	XojPdfAction();
	virtual ~XojPdfAction();

public:
	virtual XojLinkDest* getDestination() = 0;
	virtual string getTitle() = 0;

private:
	};

