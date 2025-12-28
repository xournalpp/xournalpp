#include "LinkDialog.h"

#include "control/Control.h"           // for Control
#include "gui/Builder.h"               // for Builder
#include "gui/GladeSearchpath.h"       // for GladeSearchPath
#include "util/raii/CStringWrapper.h"  // for OwnedCString

constexpr auto UI_FILE = "linkDialog.glade";
constexpr auto UI_DIALOG_NAME = "linkDialog";

void okButtonClicked(GtkButton* btn, LinkDialog* dialog) { dialog->okButtonPressed(btn); }
void cancelButtonClicked(GtkButton* btn, LinkDialog* dialog) { dialog->cancelButtonPressed(btn); }
void textChangedClb(GtkTextBuffer* buffer, LinkDialog* dialog) { dialog->textChanged(buffer); }
void layoutToogledLeft(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(LinkAlignment::LEFT); };
void layoutToogledCenter(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(LinkAlignment::CENTER); };
void layoutToogledRight(GtkToggleButton* source, LinkDialog* dialog) { dialog->layoutToggled(LinkAlignment::RIGHT); };
void urlPrefixChangedClb(GtkComboBoxText* source, LinkDialog* dialog) { dialog->urlPrefixChanged(source); };

LinkDialog::LinkDialog(Control* control, std::function<void(LinkDialog*)> callbackOK,
                       std::function<void()> callbackCancel):
        callbackOK(std::move(callbackOK)), callbackCancel(std::move(callbackCancel)) {

    Builder builder(control->getGladeSearchPath(), UI_FILE);

    this->linkDialog = static_cast<xoj::util::raii::GtkWindowUPtr>(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));
    this->textInput = GTK_TEXT_VIEW(builder.get("inpLinkEditText"));
    this->urlInput = GTK_ENTRY(builder.get("inpLinkEditURL"));
    this->okButton = GTK_BUTTON(builder.get("btnLinkEditOk"));
    this->cancelButton = GTK_BUTTON(builder.get("btnLinkEditCancel"));

    this->fontChooser = GTK_FONT_CHOOSER(builder.get("btnFontChooser"));

    this->layoutLeft = GTK_TOGGLE_BUTTON(builder.get("btnLeftLayout"));
    this->layoutCenter = GTK_TOGGLE_BUTTON(builder.get("btnCenterLayout"));
    this->layoutRight = GTK_TOGGLE_BUTTON(builder.get("btnRightLayout"));

    this->linkTypeChooser = GTK_COMBO_BOX_TEXT(builder.get("cbLinkPrefix"));
    this->urlPrefixChanged(this->linkTypeChooser);

    g_signal_connect(G_OBJECT(okButton), "clicked", G_CALLBACK(okButtonClicked), this);
    g_signal_connect(G_OBJECT(cancelButton), "clicked", G_CALLBACK(cancelButtonClicked), this);
    g_signal_connect(G_OBJECT(gtk_text_view_get_buffer(this->textInput)), "changed", G_CALLBACK(textChangedClb), this);

    g_signal_connect(G_OBJECT(layoutLeft), "released", G_CALLBACK(layoutToogledLeft), this);
    g_signal_connect(G_OBJECT(layoutCenter), "released", G_CALLBACK(layoutToogledCenter), this);
    g_signal_connect(G_OBJECT(layoutRight), "released", G_CALLBACK(layoutToogledRight), this);

    g_signal_connect(G_OBJECT(linkTypeChooser), "changed", G_CALLBACK(urlPrefixChangedClb), this);

    this->setMaxDialogHeight(control->getGtkWindow());
}

LinkDialog::~LinkDialog() = default;

void LinkDialog::preset(XojFont font, std::string text, std::string url, LinkAlignment layout) {
    GtkTextBuffer* textBuffer = gtk_text_view_get_buffer(this->textInput);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(textBuffer, &start, &end);
    gtk_text_buffer_insert(textBuffer, &start, text.c_str(), static_cast<int>(text.length()));
    gtk_text_view_set_buffer(this->textInput, textBuffer);
    URLPrefix prefix = this->identifyAndShortenURL(url);
    gtk_entry_set_text(this->urlInput, url.c_str());
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(this->linkTypeChooser), std::to_string(static_cast<int>(prefix)).c_str());
    std::string fontName = font.getName() + " " + std::to_string(font.getSize());
    gtk_font_chooser_set_font(this->fontChooser, fontName.c_str());
    this->layoutToggled(layout);
}

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
    GtkTextBuffer* text = gtk_text_view_get_buffer(this->textInput);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(text, &start, &end);
    this->linkText = std::string(gtk_text_buffer_get_text(text, &start, &end, TRUE));
    if (!this->isTextValid(this->linkText)) {
        this->linkText = "Link";
    }

    this->linkURL = std::string(gtk_entry_get_text(this->urlInput));
    if (!this->isUrlValid(this->linkURL)) {
        this->linkURL = "xournalpp.github.io";
    }
    this->callbackOK(this);
    gtk_window_close(this->linkDialog.get());
}

void LinkDialog::cancelButtonPressed(GtkButton* btn) {
    this->callbackCancel();
    gtk_window_close(this->linkDialog.get());
}

bool LinkDialog::isTextValid(const std::string& text) {
    return !std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isblank(c); });
}

bool LinkDialog::isUrlValid(const std::string& url) { return !url.empty(); }

void LinkDialog::textChanged(GtkTextBuffer* buffer) {
    gint lot = gtk_text_buffer_get_line_count(buffer);
    int width, height;
    gtk_window_get_size(this->linkDialog.get(), &width, &height);
    height = DEFAULT_HEIGHT + (std::max(0, (lot - INITIAL_NUMBER_OF_LINES + 1)) * getLineHeight());
    height = std::min(height, this->maxDialogHeight);
    gtk_window_resize(this->linkDialog.get(), width, height);
}

void LinkDialog::layoutToggled(LinkAlignment layout) {
    this->layout = layout;
    gtk_toggle_button_set_active(this->layoutLeft, layout == LinkAlignment::LEFT);
    gtk_toggle_button_set_active(this->layoutCenter, layout == LinkAlignment::CENTER);
    gtk_toggle_button_set_active(this->layoutRight, layout == LinkAlignment::RIGHT);
}

LinkAlignment LinkDialog::getLayout() { return this->layout; }

XojFont LinkDialog::getFont() {
    auto font = xoj::util::OwnedCString::assumeOwnership(gtk_font_chooser_get_font(this->fontChooser));
    return XojFont(font.get());
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

void LinkDialog::setMaxDialogHeight(GtkWindow* window) {
    GdkScreen* screen = gtk_window_get_screen(window);
    gint monitorID = gdk_screen_get_primary_monitor(screen);
    GdkRectangle monitor;
    gdk_screen_get_monitor_geometry(screen, monitorID, &monitor);
    this->maxDialogHeight = static_cast<int>(static_cast<float>(monitor.height) * MAX_HEIGHT_RATIO);
}

int LinkDialog::getLineHeight() {
    gint y, height;
    GtkTextIter iter;
    gtk_text_view_get_iter_at_location(this->textInput, &iter, 0, 0);
    gtk_text_view_get_line_yrange(this->textInput, &iter, &y, &height);
    return height;
}
