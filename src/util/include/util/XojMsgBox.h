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

#include <functional>
#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "util/raii/GtkWindowUPtr.h"

class XojMsgBox final {
public:
    XojMsgBox(
            GtkDialog* dialog, std::function<void(int)> callback = [](int) {});
    ~XojMsgBox() = default;

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    xoj::util::GtkWindowUPtr window;
    std::function<void(int)> callback;  ///< The parameter is the dialog's response ID

public:
    struct Button {
        Button(std::string l, int r): label(std::move(l)), response(r) {}
        std::string label;
        int response;
    };

    /**
     * Set window for messages without window
     */
    static void setDefaultWindow(GtkWindow* win);

    static void askQuestion(GtkWindow* win, const std::string& maintext, const std::string& secondarytext,
                            const std::vector<Button>& buttons, std::function<void(int)> callback);
    /**
     * @brief same as askQuestion() but the string maintext is not escaped for Pango markups
     */
    static void askQuestionWithMarkup(GtkWindow* win, std::string_view maintext, const std::string& secondarytext,
                                      const std::vector<Button>& buttons, std::function<void(int)> callback);

    /**
     * @brief Shows a message with title markupTitle and message content msg.
     * The title is formatted according to any Pango markups it contains.
     */
    static void showMarkupMessageToUser(GtkWindow* win, const std::string_view& markupTitle, const std::string& msg,
                                        GtkMessageType type);
    static void showMessageToUser(GtkWindow* win, const std::string& msg, GtkMessageType type);
    static void showMessageToUser(GtkWindow* win, const std::string& title, const std::string& msg,
                                  GtkMessageType type);
    static void showErrorToUser(GtkWindow* win, const std::string& msg);
    static void showErrorAndQuit(std::string& msg, int exitCode);
    static void showPluginMessage(const std::string& pluginName, const std::string& msg, bool error = false);
    [[deprecated("Will be removed when porting to gtk4")]] static int askPluginQuestion(
            const std::string& pluginName, const std::string& msg, const std::vector<Button>& buttons,
            bool error = false);
    static int replaceFileQuestion(GtkWindow* win, const std::string& msg);
    static void showHelp(GtkWindow* win);
};
