/*
 * Xournal++
 *
 * Latex implementation
 *
 * @author W Brenna
 * http://wbrenna.ca
 * 
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"

#include "model/TexImage.h"

#include <XournalType.h>

class LatexDialog : public GladeGui
{
  public:
	LatexDialog(GladeSearchpath *gladeSearchPath);
	virtual ~LatexDialog();

  public:
	virtual void show(GtkWindow *parent);
	void save();
	void load();

	//Set and retrieve text from text box
	void setTex(string texString);
	string getTex();
	
	//Set and retrieve temporary Tex render
	void setTempRender(cairo_surface_t *cairoTexTempRender);
	cairo_surface_t *getTempRender();

	//Necessary for the controller in order to connect the 'text-changed'
	//signal handler
	GtkWidget* getTexBox();

  private:
	XOJ_TYPE_ATTRIB;
	
	//Temporary render
	GtkWidget *texTempRender;
	cairo_surface_t *cairoTexTempRender;
	
	//Text field
	GtkWidget *texBox;
	string theLatex;
};
