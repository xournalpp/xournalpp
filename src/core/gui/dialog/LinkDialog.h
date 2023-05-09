/*
 * Xournal++
 *
 * Link Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for String

#include <gdk/gdk.h>  // for GdkEventKey
#include <gtk/gtk.h>  // for GtkIMContext, GtkTextIter, GtkWidget

class Control;

class LinkDialog {
public:
    LinkDialog(Control* control);
    ~LinkDialog();

public:
    void preset(std::string text, std::string url);
    int show();
    std::string getText();
    std::string getURL();

public:
    void okButtonPressed(GtkButton* btn);
    void cancelButtonPressed(GtkButton* btn);
    void textChanged(GtkTextBuffer* buffer);

private:
    bool isTextValid(std::string text);
    bool isUrlValid(std::string url);


private:
    GtkDialog* linkDialog = nullptr;

    GtkTextView* textInput = nullptr;
    GtkEntry* urlInput = nullptr;

    GtkButton* okButton = nullptr;
    GtkButton* cancelButton = nullptr;

    std::string linkText;
    std::string linkURL;

public:
    static constexpr int SUCCESS = 200;
    static constexpr int CANCEL = 400;

    static constexpr int DEFAULT_HEIGHT = 180;
    static constexpr int MAX_HEIGHT = 800;
    static constexpr int ADDITIONAL_HEIGHT_PER_LINE = 15;
    static constexpr int INITIAL_NUMBER_OF_LINES = 2;
};
