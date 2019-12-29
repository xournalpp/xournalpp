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

#include <string>

#include <gtk/gtk.h>
using std::string;
#include <map>
using std::map;

class XojMsgBox {
private:
    XojMsgBox();
    virtual ~XojMsgBox();

public:
    /**
     * Set window for messages without window
     */
    static void setDefaultWindow(GtkWindow* win);

    static void showErrorToUser(GtkWindow* win, const string& msg);
    static int showPluginMessage(const string& pluginName, const string& msg, const map<int, string>& button,
                                 bool error = false);
    static int replaceFileQuestion(GtkWindow* win, const string& msg);
    static void showHelp(GtkWindow* win);
};
