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

#include "model/Font.h"  // for XojFont

class Control;

enum class LinkAlignment { LEFT = 0, CENTER = 1, RIGHT = 2 };
enum class URLPrefix { NONE = 0, HTTP = 1, HTTPS = 2, MAILTO = 3, FILE = 4 };

class LinkDialog {
public:
    LinkDialog(Control* control);
    ~LinkDialog();

public:
    void preset(XojFont font, std::string text, std::string url, LinkAlignment layout = LinkAlignment::LEFT);
    int show();
    std::string getText();
    std::string getURL();
    LinkAlignment getLayout();
    XojFont getFont();

public:
    void okButtonPressed(GtkButton* btn);
    void cancelButtonPressed(GtkButton* btn);
    void textChanged(GtkTextBuffer* buffer);
    void layoutToggled(LinkAlignment l);
    void urlPrefixChanged(GtkComboBoxText* source);

private:
    bool isTextValid(std::string text);
    bool isUrlValid(std::string url);
    URLPrefix identifyAndShortenURL(std::string& url);

    void setMaxDialogHeight(GtkWindow* window);
    int getLineHeight();

private:
    GtkDialog* linkDialog = nullptr;

    GtkTextView* textInput = nullptr;
    GtkEntry* urlInput = nullptr;

    GtkButton* okButton = nullptr;
    GtkButton* cancelButton = nullptr;

    GtkFontChooser* fontChooser = nullptr;

    GtkToggleButton* layoutLeft = nullptr;
    GtkToggleButton* layoutCenter = nullptr;
    GtkToggleButton* layoutRight = nullptr;

    GtkComboBoxText* linkTypeChooser = nullptr;

    std::string linkText;
    std::string linkURL;
    LinkAlignment layout = LinkAlignment::LEFT;

    int maxDialogHeight = 0;

    int additionalHeightPerLine = 20;

public:
    static constexpr int SUCCESS = 200;
    static constexpr int CANCEL = 400;

    static constexpr int DEFAULT_HEIGHT = 180;
    static constexpr float MAX_HEIGHT_RATIO = 0.75;
    static constexpr int ADDITIONAL_HEIGHT_PER_LINE = 15;
    static constexpr int INITIAL_NUMBER_OF_LINES = 2;

    static constexpr char PLACEHOLDER_HTTPS[] = "xournalpp.github.io";
    static constexpr char PLACEHOLDER_MAIL[] = "email-address@provider.domain";
    static constexpr char PLACEHOLDER_FILE[] = "/absolute/path/to/file";
    static constexpr char PLACEHOLDER_OTHER[] = "https://www.google.com";
};
