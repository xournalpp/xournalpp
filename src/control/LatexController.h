/*
 * Xournal++
 *
 * Controller for Latex stuff
 *
 * @author W Brenna
 * http://wbrenna.ca
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"

#include <XournalType.h>

#include <string>
using std::string;

class Control;
class TexImage;
class Document;
class XojPageView;
class Layer;

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
	bool runCommand();

	void showTexEditDialog();
	void insertTexImage();
	void deleteOldImage();

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

	/**
	 * Image area
	 */
	int texArea;

	/**
	 * X-Position
	 */
	int posx;

	/**
	 * Y-Position
	 */
	int posy;

	/**
	 * Image width
	 */
	int imgwidth;

	/**
	 * Image height
	 */
	int imgheight;

	/**
	 * Document
	 */
	Document* doc;

	/**
	 * Page View
	 */
	XojPageView* view;

	/**
	 * Selected Page
	 */
	PageRef page;

	/**
	 * Selected layer
	 */
	Layer* layer;

	/**
	 * Image is temporary stored in configuration folder
	 */
	string texImage;

	TexImage* selectedTexImage;
};
