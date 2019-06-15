/*
 * Xournal++
 *
 * The definitions for the possibel toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

typedef struct
{
	/**
	 * The name in the glade.xml file
	 */
	const char* guiName;

	/**
	 * The name in the .ini file
	 */
	const char* propName;

	/**
	 * if horizontal (true) or vertical (false)
	 */
	bool horizontal;
} ToolbarEntryDefintion;

const static ToolbarEntryDefintion TOOLBAR_DEFINITIONS[] = {
	{ "tbTop1",    "toolbarTop1",    true  },
	{ "tbTop2",    "toolbarTop2",    true  },
	{ "tbLeft1",   "toolbarLeft1",   false },
	{ "tbLeft2",   "toolbarLeft2",   false },
	{ "tbRight1",  "toolbarRight1",  false },
	{ "tbRight2",  "toolbarRight2",  false },
	{ "tbBottom1", "toolbarBottom1", true  },
	{ "tbBottom2", "toolbarBottom2", true  },
	{ "tbFloat1", "toolbarFloat1", true },
	{ "tbFloat2", "toolbarFloat2", true },
	{ "tbFloat3", "toolbarFloat3", true },
	{ "tbFloat4", "toolbarFloat4", true }
};


const static int TOOLBAR_DEFINITIONS_LEN = G_N_ELEMENTS(TOOLBAR_DEFINITIONS);


//Used in FloatingToolbox.cpp to check if the Toolbox isempty.
const static ToolbarEntryDefintion  FLOATINGTOOLBOX_TOOLBARS[] = {
	{ "tbFloat1", "toolbarFloat1", true },
	{ "tbFloat2", "toolbarFloat2", true },
	{ "tbFloat3", "toolbarFloat3", true },
	{ "tbFloat4", "toolbarFloat4", true }
};

const static int FLOATINGTOOLBOX_TOOLBARS_LEN = G_N_ELEMENTS(FLOATINGTOOLBOX_TOOLBARS);
