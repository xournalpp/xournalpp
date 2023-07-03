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
gboolean drawFtColorIconInternal(GtkWidget* src, cairo_t* cr, TextEditorContextMenu* tecm) {
    return tecm->drawFtColorIcon(src, cr);
};
gboolean drawBgColorIconInternal(GtkWidget* src, cairo_t* cr, TextEditorContextMenu* tecm) {
    return tecm->drawBgColorIcon(src, cr);
};

void tglBoldStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleBoldStyle(); }
void tglItalicStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleItalicStyle(); }
void tglUnderlineStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleUnderlineStyle(); }


TextEditorContextMenu::TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView,
                                             GtkWidget* xournalWidget):
        control(control), editor(editor), pageView(pageView), xournalWidget(xournalWidget) {
    // Only for debugging
    std::cout << "TextEditorContextMenu created!" << std::endl;
    this->create();
}

TextEditorContextMenu::~TextEditorContextMenu() {
    gtk_widget_destroy(GTK_WIDGET(this->fontBtn));
    // gtk_widget_destroy(GTK_WIDGET(this->ftColorIcon));
    gtk_widget_destroy(GTK_WIDGET(this->ftColorBtn));
    // gtk_widget_destroy(GTK_WIDGET(this->bgColorIcon));
    gtk_widget_destroy(GTK_WIDGET(this->bgColorBtn));
    gtk_widget_destroy(GTK_WIDGET(this->alignLeftTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignCenterTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignRightTgl));
    gtk_popover_set_relative_to(this->contextMenu, NULL);  // Destroys popover and frees memory
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
    gtk_popover_set_modal(this->contextMenu, false);
    gtk_widget_set_can_focus(GTK_WIDGET(this->contextMenu), false);

    this->fontBtn = GTK_FONT_BUTTON(gtk_builder_get_object(builder, "btnFontChooser"));
    g_signal_connect(this->fontBtn, "font-set", G_CALLBACK(changeFontInternal), this);


    this->tglBoldBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoBold"));
    this->tglItalicBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoItalic"));
    this->tglUnderlineBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoUnderline"));
    this->expandTextDecoration = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoExpand"));
    g_signal_connect(tglBoldBtn, "clicked", G_CALLBACK(tglBoldStyle), this);
    g_signal_connect(tglItalicBtn, "clicked", G_CALLBACK(tglItalicStyle), this);
    g_signal_connect(tglUnderlineBtn, "clicked", G_CALLBACK(tglUnderlineStyle), this);

    this->ftColorBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnFontColor"));
    this->bgColorBtn = GTK_BUTTON(gtk_builder_get_object(builder, "btnBgColor"));
    g_signal_connect(this->ftColorBtn, "clicked", G_CALLBACK(changeFtColorInternal), this);
    g_signal_connect(this->bgColorBtn, "clicked", G_CALLBACK(changeBgColorInternal), this);

    this->ftColorIcon = GTK_WIDGET(gtk_builder_get_object(builder, "imgFtColor"));
    g_signal_connect(this->ftColorIcon, "draw", G_CALLBACK(drawFtColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->ftColorBtn), this->ftColorIcon);

    this->bgColorIcon = GTK_WIDGET(gtk_builder_get_object(builder, "imgBgColor"));
    g_signal_connect(this->bgColorIcon, "draw", G_CALLBACK(drawBgColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->bgColorBtn), this->bgColorIcon);

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
    this->editor->setFontInline(desc);
    pango_font_description_free(desc);
    gtk_widget_grab_focus(this->xournalWidget);
}

void TextEditorContextMenu::changeFtColor() {
    GtkWidget* dialog = gtk_color_chooser_dialog_new("Foreground Color", control->getGtkWindow());
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), FALSE);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &ftColor);
    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_OK) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &ftColor);
        this->editor->setFontColorInline(ftColor);
        std::cout << "New font color: (" << ftColor.red << ";" << ftColor.green << ";" << ftColor.blue << ")"
                  << std::endl;
    }
    gtk_widget_grab_focus(this->xournalWidget);
    gtk_widget_destroy(dialog);
}

void TextEditorContextMenu::changeBgColor() {
    GtkWidget* dialog = gtk_color_chooser_dialog_new("Background Color", control->getGtkWindow());
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), TRUE);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &bgColor);
    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_OK) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &bgColor);
        this->editor->setBackgroundColorInline(bgColor);
        std::cout << "New bg color: (" << bgColor.red << ";" << bgColor.green << ";" << bgColor.blue << ")"
                  << std::endl;
    }
    gtk_widget_grab_focus(this->xournalWidget);
    gtk_widget_destroy(dialog);
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
    gtk_widget_grab_focus(this->xournalWidget);
}

gboolean TextEditorContextMenu::drawFtColorIcon(GtkWidget* src, cairo_t* cr) {
    guint width, height;
    GtkStyleContext* context;
    context = gtk_widget_get_style_context(src);
    width = gtk_widget_get_allocated_width(src);
    height = gtk_widget_get_allocated_height(src);
    gtk_render_background(context, cr, 0, 0, width, height);

    // Draw font icon here
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0, height - COLOR_BAR_HEIGHT, width, COLOR_BAR_HEIGHT);

    gdk_cairo_set_source_rgba(cr, &ftColor);

    cairo_fill(cr);
    return FALSE;
}

gboolean TextEditorContextMenu::drawBgColorIcon(GtkWidget* src, cairo_t* cr) {
    guint width, height;
    GtkStyleContext* context;
    context = gtk_widget_get_style_context(src);
    width = gtk_widget_get_allocated_width(src);
    height = gtk_widget_get_allocated_height(src);
    gtk_render_background(context, cr, 0, 0, width, height);

    // Draw font icon here
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0, height - COLOR_BAR_HEIGHT, width, COLOR_BAR_HEIGHT);

    gdk_cairo_set_source_rgba(cr, &bgColor);

    cairo_fill(cr);
    return FALSE;
}

void TextEditorContextMenu::toggleBoldStyle() {
    std::cout << "Bold" << std::endl;
    this->editor->addTextAttributeInline(pango_attr_weight_new(PANGO_WEIGHT_BOLD));
}

void TextEditorContextMenu::toggleItalicStyle() {
    std::cout << "Italic" << std::endl;
    this->editor->addTextAttributeInline(pango_attr_style_new(PANGO_STYLE_ITALIC));
}

void TextEditorContextMenu::toggleUnderlineStyle() {
    std::cout << "Underline" << std::endl;
    this->editor->addTextAttributeInline(pango_attr_underline_new(PANGO_UNDERLINE_SINGLE));
}
