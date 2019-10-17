/*
 * Xournal++
 *
 * Interface for touch disable implementations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "TouchDisableInterface.h"


class TouchDisableCustom : public TouchDisableInterface
{
public:
	TouchDisableCustom(string enableCommand, string disableCommand);
	virtual ~TouchDisableCustom();

public:
	virtual void enableTouch();
	virtual void disableTouch();

private:
	string enableCommand;
	string disableCommand;
};
