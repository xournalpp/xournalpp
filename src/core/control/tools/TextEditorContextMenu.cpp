#include "TextEditorContextMenu.h"

#include <iostream>
#include <string>

#include "control/Control.h"                // for Control
#include "gui/GladeSearchpath.h"            // for GladeSearchPath
#include "gui/PageView.h"                   // for PageView
#include "model/Text.h"                     // for Text
#include "view/overlays/TextEditionView.h"  // for TextEditionView

#include "TextEditor.h"  // for TextEditor

void changeFontInternal(GtkFontButton* src, TextEditorContextMenu* tecm) { tecm->changeFont(); }
void changeFtColorInternal(GtkColorButton* src, TextEditorContextMenu* tecm) { tecm->changeFtColor(); }
void changeBgColorInternal(GtkColorButton* src, TextEditorContextMenu* tecm) { tecm->changeBgColor(); }
void toggleAlignLeft(GtkButton* src, TextEditorContextMenu* tecm) { tecm->changeAlignment(TextAlignment::LEFT); }
void toggleAlignCenter(GtkButton* src, TextEditorContextMenu* tecm) { tecm->changeAlignment(TextAlignment::CENTER); }
void toggleAlignRight(GtkButton* src, TextEditorContextMenu* tecm) { tecm->changeAlignment(TextAlignment::RIGHT); }

TextEditorContextMenu::TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView,
                                             GtkWidget* xournalWidget):
        control(control), editor(editor), pageView(pageView), xournalWidget(xournalWidget) {
    // Only for debugging
    std::cout << "TextEditorContextMenu created!" << std::endl;
    this->create();
}

TextEditorContextMenu::~TextEditorContextMenu() {
    gtk_widget_destroy(GTK_WIDGET(this->contextMenu));
    /*gtk_widget_destroy(GTK_WIDGET(this->fontBtn));
    gtk_widget_destroy(GTK_WIDGET(this->ftColorBtn));
    gtk_widget_destroy(GTK_WIDGET(this->bgColorBtn));
    gtk_widget_destroy(GTK_WIDGET(this->alignLeftTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignCenterTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignRightTgl));*/
    std::cout << "TextEditorContextMenu destroyed!" << std::endl;
}

void TextEditorContextMenu::show() {
    this->reposition();
    gtk_widget_show_all(GTK_WIDGET(this->contextMenu));
    gtk_popover_popup(this->contextMenu);
    std::cout << "Popup menu should be shown" << std::endl;
}

void TextEditorContextMenu::hide() {
    gtk_popover_popdown(this->contextMenu);
    std::cout << "Popup menu should be hidden" << std::endl;
}

void TextEditorContextMenu::reposition() {
    int padding = xoj::view::TextEditionView::PADDING_IN_PIXELS;
    Range r = this->editor->getContentBoundingBox();
    GdkRectangle rect{this->pageView->getX() + int(r.getX() * this->pageView->getZoom()),
                      this->pageView->getY() + int(r.getY() * this->pageView->getZoom()) - padding,
                      int(r.getWidth() * this->pageView->getZoom()), int(r.getHeight() * this->pageView->getZoom())};
    gtk_popover_set_pointing_to(this->contextMenu, &rect);
}

void TextEditorContextMenu::create() {
    auto filepath = this->control->getGladeSearchPath()->findFile("", "textEditorContextMenu.glade");

    GtkBuilder* builder = gtk_builder_new();

    GError* err = NULL;
    if (gtk_builder_add_from_file(builder, filepath.u8string().c_str(), &err) == 0) {
        std::cout << err->message << std::endl;
    }

    this->contextMenu = GTK_POPOVER(gtk_builder_get_object(builder, "textEditorContextMenu"));
    gtk_popover_set_relative_to(this->contextMenu, this->xournalWidget);
    gtk_popover_set_constrain_to(this->contextMenu, GTK_POPOVER_CONSTRAINT_WINDOW);

    this->fontBtn = GTK_FONT_BUTTON(gtk_builder_get_object(builder, "btnFontChooser"));
    g_signal_connect(this->fontBtn, "font-set", G_CALLBACK(changeFontInternal), this);

    this->ftColorBtn = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "btnFontColor"));
    this->bgColorBtn = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "btnBgColor"));
    g_signal_connect(this->ftColorBtn, "color-set", G_CALLBACK(changeFtColorInternal), this);
    g_signal_connect(this->bgColorBtn, "color-set", G_CALLBACK(changeBgColorInternal), this);

    this->alignLeftTgl = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignLeft"));
    this->alignCenterTgl = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignCenter"));
    this->alignRightTgl = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignRight"));
    g_signal_connect(this->alignLeftTgl, "released", G_CALLBACK(toggleAlignLeft), this);
    g_signal_connect(this->alignCenterTgl, "released", G_CALLBACK(toggleAlignCenter), this);
    g_signal_connect(this->alignRightTgl, "released", G_CALLBACK(toggleAlignRight), this);

    g_object_unref(G_OBJECT(builder));
}

void TextEditorContextMenu::changeFont() {
    PangoFontDescription* desc = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(this->fontBtn));
    std::string fontDesc(pango_font_description_to_string(desc));
    std::string fontName(pango_font_description_get_family(desc));
    std::string fontSize(std::to_string(pango_font_description_get_size(desc)));
    std::cout << fontDesc << ":" << fontName << ":" << fontSize << std::endl;
    XojFont font;
    font = fontDesc;
    this->editor->setFont(font);
    pango_font_description_free(desc);
}

void TextEditorContextMenu::changeFtColor() {
    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(this->ftColorBtn), &color);
    this->editor->setColor(Color(color.red * 255, color.green * 255, color.blue * 255, color.alpha * 255));
    std::cout << "New font color: (" << color.red << ";" << color.green << ";" << color.blue << ")" << std::endl;
}

void TextEditorContextMenu::changeBgColor() {
    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(this->ftColorBtn), &color);
    std::cout << "New background color: (" << color.red << ";" << color.green << ";" << color.blue << ")" << std::endl;
}

void TextEditorContextMenu::changeAlignment(TextAlignment align) {
    switch (align) {
        case TextAlignment::LEFT:
            gtk_toggle_button_set_active(this->alignLeftTgl, true);
            gtk_toggle_button_set_active(this->alignCenterTgl, false);
            gtk_toggle_button_set_active(this->alignRightTgl, false);
            break;
        case TextAlignment::CENTER:
            gtk_toggle_button_set_active(this->alignLeftTgl, false);
            gtk_toggle_button_set_active(this->alignCenterTgl, true);
            gtk_toggle_button_set_active(this->alignRightTgl, false);
            break;
        case TextAlignment::RIGHT:
            gtk_toggle_button_set_active(this->alignLeftTgl, false);
            gtk_toggle_button_set_active(this->alignCenterTgl, false);
            gtk_toggle_button_set_active(this->alignRightTgl, true);
            break;
        default:
            gtk_toggle_button_set_active(this->alignLeftTgl, false);
            gtk_toggle_button_set_active(this->alignCenterTgl, false);
            gtk_toggle_button_set_active(this->alignRightTgl, false);
            break;
    }
    this->editor->setTextAlignment(align);
}
