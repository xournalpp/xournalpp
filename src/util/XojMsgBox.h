/*
 * Xournal++
 *
 * Xournal util functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include <string>
using std::string;

enum MsgBoxButtonType {
	MSG_BT_OK     = 1 << 0,
	MSG_BT_YES    = 1 << 1,
	MSG_BT_NO     = 1 << 2,
	MSG_BT_CANCEL = 1 << 3
};

class XojMsgBox
{
private:
	XojMsgBox();
	virtual ~XojMsgBox();

public:
	/**
	 * Set window for messages without window
	 */
	static void setDefaultWindow(GtkWindow* win);

	static void showErrorToUser(GtkWindow* win, string msg);
	static int showPluginMessage(string pluginName, string msg, int type, bool error = false);
	static int replaceFileQuestion(GtkWindow* win, string msg);
	static void showHelp(GtkWindow* win);
};
