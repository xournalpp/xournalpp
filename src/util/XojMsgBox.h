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

#include <map>
#include <string>

#include <gtk/gtk.h>

class XojMsgBox {
private:
    XojMsgBox();
    virtual ~XojMsgBox();

public:
    /**
     * Set window for messages without window
     */
    static void setDefaultWindow(GtkWindow* win);

    static void showErrorToUser(GtkWindow* win, const std::string& msg);
    static int showPluginMessage(const std::string& pluginName, const std::string& msg,
                                 const std::map<int, std::string>& button, bool error = false);
    static int replaceFileQuestion(GtkWindow* win, const std::string& msg);
    static void showHelp(GtkWindow* win);
};
