/*
 * Xournal++
 *
 * The definitions for the possibel toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


typedef struct {
	/**
	 * The name in the glade.xml file
	 */
	const char * guiName;

	/**
	 * The name in the .ini file
	 */
	const char * propName;

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
		{ "tbBottom2", "toolbarBottom2", true  }
};

const static int TOOLBAR_DEFINITIONS_LEN = G_N_ELEMENTS(TOOLBAR_DEFINITIONS);

