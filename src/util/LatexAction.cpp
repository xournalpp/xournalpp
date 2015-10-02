#include "LatexAction.h"

#include "control/Control.h"
#include "control/tools/ImageHandler.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Stroke.h"
#include "model/Layer.h"
#include "model/TexImage.h"
//TODO some time - clean up these includes
#include "serializing/HexObjectEncoding.h"
#include "serializing/ObjectOutputStream.h"

#include <i18n.h>

#include <glib.h>

#include <iostream>
using namespace std;

LatexAction::LatexAction(string myTex, double tArea)
{
	//this->control = control;

	this->theLatex = myTex;

	this->texfile = Util::getConfigFile("tex").string();
	this->texfilefull = Util::getConfigFile("tex.png").string();

	cout << this->texfile << endl;

	//set up the default positions.
	this->myx = 0;
	this->myy = 0;

	this->texArea = tArea;
}

void LatexAction::runCommand()
{
	/*
	 * at some point, I may need to sanitize theLatex
	 */
	cout << C_("LaTeX comand", "Command is being run.") << endl;
	const gchar* mtex = "mathtex-xournalpp.cgi";
	gchar* mathtex = g_find_program_in_path(mtex);
	if (!mathtex)
	{
		cerr << _("Error: problem finding mathtex. Doing nothing…") << endl;
		return;
	}
	cout << _F("Found mathtex in your path! TeX area is {1}") % this->texArea << endl;
	g_free(mathtex);
	//can change font colour later with more features
	string fontcolour = "black";
	//dpi 300 is a good balance
	string texres;
	if (this->texArea < 1000)
	{
		texres = "300";
	}
	else if (this->texArea < 4000)
	{
		texres = "400";
	}
	else if (this->texArea < 8000)
	{
		texres = "500";
	}
	else if (this->texArea < 16000)
	{
		texres = "600";
	}
	else if (this->texArea < 32000)
	{
		texres = "800";
	}
	else
	{
		texres = "1000";
	}
	string command = FS(bl::format("{1} -m 0 \"\\png\\usepackage{{color}}\\color{{{2}}}\\dpi{{{3}}}\\normalsize {4}\" -o {5}")
						% mtex % (fontcolour.length() ? fontcolour : "black") % texres
						% g_strescape(this->theLatex.c_str(), NULL) % this->texfile);

	gint rt = 0;
	void(*texhandler)(int) = signal(SIGCHLD, SIG_DFL);
	gboolean success = g_spawn_command_line_sync(command.c_str(), NULL, NULL, &rt, NULL);
	signal(SIGCHLD, texhandler);
	if (!success)
	{
		cout << _("LaTeX command execution failed.") << endl;
		return;
	}
	cout << _F("LaTeX command: \"{1}\" executed successfully. Result saved to file {2}.") % this->theLatex % this->texfilefull << endl;

}

string LatexAction::getFileName()
{
	//gets the filename for our image
	return this->texfilefull;
}
