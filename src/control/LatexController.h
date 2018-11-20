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
	/**
	 * Find the tex executable, return false if not found
	 */
	bool findTexExecutable();

	/**
	 * Find a selected tex element, and load it
	 */
	void findSelectedTexElement();

	/**
	 * Run LaTeX Command
	 */
	void runCommand();

	void showTexEditDialog();

private:
	XOJ_TYPE_ATTRIB;

	Control* control;

	/**
	 * Tex binary full path
	 */
	string binTex;

	/**
	 * Orignal TeX, if editing
	 */
	string initalTex;

	/**
	 * Updated TeX string
	 */
	string currentTex;
};
