#include "LinkDialog.h"

#include "control/Control.h"      // for Control
#include "gui/GladeSearchpath.h"  // for GladeSearchPath

void okButtonClicked(GtkButton* btn, LinkDialog* dialog) { dialog->okButtonPressed(btn); }
void cancelButtonClicked(GtkButton* btn, LinkDialog* dialog) { dialog->cancelButtonPressed(btn); }
void textChangedClb(GtkTextBuffer* buffer, LinkDialog* dialog) { dialog->textChanged(buffer); }
void layoutToogledLeft(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(Layout::LEFT); };
void layoutToogledCenter(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(Layout::CENTER); };
void layoutToogledRight(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(Layout::RIGHT); };
void urlPrefixChangedClb(GtkComboBoxText* source, LinkDialog* dialog) { dialog->urlPrefixChanged(source); };

LinkDialog::LinkDialog(Control* control) {
    auto filepath = control->getGladeSearchPath()->findFile("", "linkDialog.glade");

    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, filepath.u8string().c_str(), NULL);

    this->linkDialog = GTK_DIALOG(gtk_builder_get_object(builder, "linkDialog"));
    this->textInput = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "inpLinkEditText"));
    this->urlInput = GTK_ENTRY(gtk_builder_get_object(builder, "inpLinkEditURL"));
    this->okButton = GTK_BUTTON(gtk_builder_get_object(builder, "btnLinkEditOk"));
    this->cancelButton = GTK_BUTTON(gtk_builder_get_object(builder, "btnLinkEditCancel"));

    this->fontChooser = GTK_FONT_CHOOSER(gtk_builder_get_object(builder, "btnFontChooser"));

    this->layoutLeft = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnLeftLayout"));
    this->layoutCenter = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnCenterLayout"));
    this->layoutRight = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnRightLayout"));

    this->linkTypeChooser = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "cbLinkPrefix"));
    this->urlPrefixChanged(this->linkTypeChooser);

    g_signal_connect(G_OBJECT(okButton), "clicked", G_CALLBACK(okButtonClicked), this);
    g_signal_connect(G_OBJECT(cancelButton), "clicked", G_CALLBACK(cancelButtonClicked), this);
    g_signal_connect(G_OBJECT(gtk_text_view_get_buffer(this->textInput)), "changed", G_CALLBACK(textChangedClb), this);

    g_signal_connect(G_OBJECT(layoutLeft), "released", G_CALLBACK(layoutToogledLeft), this);
    g_signal_connect(G_OBJECT(layoutCenter), "released", G_CALLBACK(layoutToogledCenter), this);
    g_signal_connect(G_OBJECT(layoutRight), "released", G_CALLBACK(layoutToogledRight), this);

    g_signal_connect(G_OBJECT(linkTypeChooser), "changed", G_CALLBACK(urlPrefixChangedClb), this);
}

LinkDialog::~LinkDialog() { gtk_widget_destroy(GTK_WIDGET(linkDialog)); }

void LinkDialog::preset(XojFont font, std::string text, std::string url, Layout layout) {
    GtkTextBuffer* textBuffer = gtk_text_view_get_buffer(this->textInput);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(textBuffer, &start, &end);
    gtk_text_buffer_insert(textBuffer, &start, text.c_str(), text.length());
    gtk_text_view_set_buffer(this->textInput, textBuffer);
    URLPrefix prefix = this->identifyAndShortenURL(url);
    gtk_entry_set_text(this->urlInput, url.c_str());
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(this->linkTypeChooser), std::to_string(static_cast<int>(prefix)).c_str());
    std::string fontName = font.getName() + " " + std::to_string(font.getSize());
    gtk_font_chooser_set_font(this->fontChooser, fontName.c_str());
    this->layoutToggled(layout);
}

int LinkDialog::show() { return gtk_dialog_run(GTK_DIALOG(this->linkDialog)); }

std::string LinkDialog::getText() { return this->linkText; }

std::string LinkDialog::getURL() {
    std::string res = gtk_combo_box_text_get_active_text(this->linkTypeChooser);
    if (res == "<custom>") {
        res = "";
    }
    res = res + this->linkURL;
    return res;
}

void LinkDialog::okButtonPressed(GtkButton* btn) {
    std::cout << "OK Button pressed" << std::endl;
    GtkTextBuffer* text = gtk_text_view_get_buffer(this->textInput);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(text, &start, &end);
    this->linkText = std::string(gtk_text_buffer_get_text(text, &start, &end, TRUE));
    std::cout << "  Link Text: " << this->linkText << std::endl;
    if (!this->isTextValid(this->linkText)) {
        this->linkText = "Link";
    }

    this->linkURL = std::string(gtk_entry_get_text(this->urlInput));
    std::cout << "  Link URL: " << this->linkURL << std::endl;
    if (!this->isUrlValid(this->linkURL)) {
        this->linkURL = "xournalpp.github.io";
    }

    gtk_dialog_response(linkDialog, LinkDialog::SUCCESS);
}

void LinkDialog::cancelButtonPressed(GtkButton* btn) {
    std::cout << "Cancel Button pressed" << std::endl;
    gtk_dialog_response(linkDialog, LinkDialog::CANCEL);
}


bool LinkDialog::isTextValid(std::string text) {
    if (text.empty()) {
        return false;
    }

    int nBlankChars = 0;
    for (char c: text) {
        if (std::isblank(c)) {
            nBlankChars++;
        }
    }

    if (text.length() == nBlankChars) {
        return false;
    }

    return true;
}

bool LinkDialog::isUrlValid(std::string url) {
    if (url.empty()) {
        return false;
    }
    return true;
}


void LinkDialog::textChanged(GtkTextBuffer* buffer) {
    gint lot = gtk_text_buffer_get_line_count(buffer);
    int width, height;
    gtk_window_get_size(GTK_WINDOW(this->linkDialog), &width, &height);
    height = DEFAULT_HEIGHT + (std::max(0, (lot - INITIAL_NUMBER_OF_LINES)) * ADDITIONAL_HEIGHT_PER_LINE);
    height = std::min(height, MAX_HEIGHT);
    gtk_window_resize(GTK_WINDOW(this->linkDialog), width, height);
}

void LinkDialog::layoutToggled(Layout layout) {
    this->layout = layout;
    if (layout == Layout::LEFT) {
        gtk_toggle_button_set_active(this->layoutLeft, true);
        gtk_toggle_button_set_active(this->layoutCenter, false);
        gtk_toggle_button_set_active(this->layoutRight, false);
    } else if (layout == Layout::CENTER) {
        gtk_toggle_button_set_active(this->layoutLeft, false);
        gtk_toggle_button_set_active(this->layoutCenter, true);
        gtk_toggle_button_set_active(this->layoutRight, false);
    } else if (layout == Layout::RIGHT) {
        gtk_toggle_button_set_active(this->layoutLeft, false);
        gtk_toggle_button_set_active(this->layoutCenter, false);
        gtk_toggle_button_set_active(this->layoutRight, true);
    }
}

Layout LinkDialog::getLayout() { return this->layout; }

XojFont LinkDialog::getFont() {
    XojFont newfont;
    std::string fontName = gtk_font_chooser_get_font(this->fontChooser);
    auto pos = fontName.find_last_of(" ");
    newfont.setName(fontName.substr(0, pos));
    newfont.setSize(std::stod(fontName.substr(pos + 1)));
    return newfont;
}

void LinkDialog::urlPrefixChanged(GtkComboBoxText* eventSource) {
    URLPrefix prefix = static_cast<URLPrefix>(std::stoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(eventSource))));
    switch (prefix) {
        case URLPrefix::NONE:
            gtk_entry_set_placeholder_text(this->urlInput, PLACEHOLDER_OTHER);
            break;
        case URLPrefix::HTTP:
        case URLPrefix::HTTPS:
            gtk_entry_set_placeholder_text(this->urlInput, PLACEHOLDER_HTTPS);
            break;
        case URLPrefix::MAILTO:
            gtk_entry_set_placeholder_text(this->urlInput, PLACEHOLDER_MAIL);
            break;
        case URLPrefix::FILE:
            gtk_entry_set_placeholder_text(this->urlInput, PLACEHOLDER_FILE);
            break;
        default:
            break;
    }
}

URLPrefix LinkDialog::identifyAndShortenURL(std::string& url) {
    if (url.rfind("https://", 0) == 0) {
        url = url.substr(8);
        return URLPrefix::HTTPS;
    } else if (url.rfind("http://", 0) == 0) {
        url = url.substr(7);
        return URLPrefix::HTTP;
    } else if (url.rfind("mailto:", 0) == 0) {
        url = url.substr(7);
        return URLPrefix::MAILTO;
    } else if (url.rfind("file://", 0) == 0) {
        url = url.substr(7);
        return URLPrefix::FILE;
    } else {
        return URLPrefix::NONE;
    }
}
