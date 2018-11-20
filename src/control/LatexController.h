/*
 * Xournal++
 *
 * Controller for Latex stuff
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <string>
using std::string;

class Control;

class LatexController
{
public:
	LatexController(Control* control);
	virtual ~LatexController();

public:
	void run();

private:
	string deleteSelectedTexImag();
	string showTexEditDialog(string tex);

private:
	XOJ_TYPE_ATTRIB;

	Control* control;
};
