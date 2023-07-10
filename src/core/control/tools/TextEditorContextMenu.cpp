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
void tglSecToolbar(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleSecondaryToolbar(); }

void tglBoldStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleWeight(PANGO_WEIGHT_BOLD); }
void tglItalicStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleStyle(PANGO_STYLE_ITALIC); }
void tglUnderlineStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleUnderline(PANGO_UNDERLINE_SINGLE); }


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
    if (!isVisible) {
        this->reposition();
        this->showReducedMenu();
        gtk_popover_popup(this->contextMenu);
        isVisible = true;
        std::cout << "Popup menu should be shown" << std::endl;
    }
}

void TextEditorContextMenu::hide() {
    if (isVisible) {
        gtk_popover_popdown(this->contextMenu);
        isVisible = false;
        std::cout << "Popup menu should be hidden" << std::endl;
    }
}

void TextEditorContextMenu::showReducedMenu() {
    gtk_widget_set_visible(GTK_WIDGET(this->fontBtn), false);
    gtk_widget_set_visible(this->textDecoLayout, false);
    gtk_widget_set_visible(this->colorLayout, false);
    gtk_widget_set_visible(this->alignmentLayout, true);
    gtk_widget_set_visible(this->secondaryToolbar, false);
}

void TextEditorContextMenu::showFullMenu() {
    gtk_widget_set_visible(GTK_WIDGET(this->fontBtn), true);
    gtk_widget_set_visible(this->textDecoLayout, true);
    gtk_widget_set_visible(this->colorLayout, true);
    gtk_widget_set_visible(this->alignmentLayout, true);
    gtk_widget_set_visible(this->secondaryToolbar, false);
}

void TextEditorContextMenu::reposition() {
    int padding = xoj::view::TextEditionView::PADDING_IN_PIXELS;
    Range r = this->editor->getContentBoundingBox();
    GdkRectangle rect{this->pageView->getX() + int(r.getX() * this->pageView->getZoom()),
                      this->pageView->getY() + int(r.getY() * this->pageView->getZoom()) - padding,
                      int(r.getWidth() * this->pageView->getZoom()), int(r.getHeight() * this->pageView->getZoom())};
    gtk_popover_set_pointing_to(this->contextMenu, &rect);
}

void TextEditorContextMenu::toggleSecondaryToolbar() {
    if (gtk_widget_is_visible(this->secondaryToolbar)) {
        std::cout << "Show secondary toolbar!" << std::endl;
        gtk_widget_set_visible(this->secondaryToolbar, false);
    } else {
        std::cout << "Hide secondary toolbar!" << std::endl;
        gtk_widget_set_visible(this->secondaryToolbar, true);
    }
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
    gtk_widget_hide(GTK_WIDGET(this->contextMenu));

    this->fontBtn = GTK_FONT_BUTTON(gtk_builder_get_object(builder, "btnFontChooser"));
    g_signal_connect(this->fontBtn, "font-set", G_CALLBACK(changeFontInternal), this);


    this->tglBoldBtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnDecoBold"));
    this->tglItalicBtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnDecoItalic"));
    this->tglUnderlineBtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnDecoUnderline"));
    this->expandTextDecoration = GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoExpand"));
    g_signal_connect(tglBoldBtn, "released", G_CALLBACK(tglBoldStyle), this);
    g_signal_connect(tglItalicBtn, "released", G_CALLBACK(tglItalicStyle), this);
    g_signal_connect(tglUnderlineBtn, "released", G_CALLBACK(tglUnderlineStyle), this);
    g_signal_connect(expandTextDecoration, "clicked", G_CALLBACK(tglSecToolbar), this);

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

    this->textDecoLayout = GTK_WIDGET(gtk_builder_get_object(builder, "textDecoLayout"));
    this->colorLayout = GTK_WIDGET(gtk_builder_get_object(builder, "colorLayout"));
    this->alignmentLayout = GTK_WIDGET(gtk_builder_get_object(builder, "alignmentLayout"));
    this->secondaryToolbar = GTK_WIDGET(gtk_builder_get_object(builder, "secondaryToolbar"));

    g_object_unref(G_OBJECT(builder));
}

void TextEditorContextMenu::changeFont() {
    PangoFontDescription* desc = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(this->fontBtn));
    std::string fontDesc(pango_font_description_to_string(desc));
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

void TextEditorContextMenu::toggleWeight(PangoWeight weight) {
    std::cout << "Bold" << std::endl;
    if (this->weight = weight) {
        this->editor->addTextAttributeInline(pango_attr_weight_new(PANGO_WEIGHT_NORMAL));
        this->switchWeightButtons(PANGO_WEIGHT_NORMAL);
    } else {
        this->editor->addTextAttributeInline(pango_attr_weight_new(weight));
        this->switchWeightButtons(weight);
    }
}

void TextEditorContextMenu::toggleStyle(PangoStyle style) {
    std::cout << "Italic" << std::endl;
    if (this->style = style) {
        this->editor->addTextAttributeInline(pango_attr_style_new(PANGO_STYLE_NORMAL));
        this->switchStyleButtons(PANGO_STYLE_NORMAL);
    } else {
        this->editor->addTextAttributeInline(pango_attr_style_new(style));
        this->switchStyleButtons(style);
    }
}

void TextEditorContextMenu::toggleUnderline(PangoUnderline underline) {
    std::cout << "Underline" << std::endl;
    if (this->underline == underline) {
        this->editor->addTextAttributeInline(pango_attr_underline_new(PANGO_UNDERLINE_NONE));
        this->switchUnderlineButtons(PANGO_UNDERLINE_NONE);
    } else {
        this->editor->addTextAttributeInline(pango_attr_underline_new(underline));
        this->switchUnderlineButtons(underline);
    }
}

void TextEditorContextMenu::setAttributes(std::list<PangoAttribute*> attributes) {
    this->clearAttributes();
    for (PangoAttribute* a: attributes) {
        this->attributes.push_back(a);
    }
    this->applyAttributes();
}

void TextEditorContextMenu::clearAttributes() {
    for (PangoAttribute* p: this->attributes) {
        pango_attribute_destroy(p);
    }
    this->attributes.clear();
}

void TextEditorContextMenu::applyAttributes() {
    for (PangoAttribute* p: this->attributes) {
        switch (p->klass->type) {
            case PANGO_ATTR_FONT_DESC: {
                PangoAttrFontDesc* desc = pango_attribute_as_font_desc(p);
                gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(this->fontBtn), desc->desc);
                break;
            }
            case PANGO_ATTR_FOREGROUND: {
                PangoAttrColor* ftColor = pango_attribute_as_color(p);
                this->ftColor.red = double(ftColor->color.red) / double(UINT16_MAX);
                this->ftColor.green = double(ftColor->color.green) / double(UINT16_MAX);
                this->ftColor.blue = double(ftColor->color.blue) / double(UINT16_MAX);
                this->ftColor.alpha = 1.0;
                break;
            }
            case PANGO_ATTR_BACKGROUND: {
                PangoAttrColor* bgColor = pango_attribute_as_color(p);
                this->bgColor.red = double(bgColor->color.red) / double(UINT16_MAX);
                this->bgColor.green = double(bgColor->color.green) / double(UINT16_MAX);
                this->bgColor.alpha = 1.0;
                break;
            }
            case PANGO_ATTR_STYLE: {
                PangoAttrInt* style = pango_attribute_as_int(p);
                switchStyleButtons(static_cast<PangoStyle>(style->value));
                break;
            }
            case PANGO_ATTR_WEIGHT: {
                PangoAttrInt* weight = pango_attribute_as_int(p);
                switchWeightButtons(static_cast<PangoWeight>(weight->value));
                break;
            }
            case PANGO_ATTR_UNDERLINE: {
                PangoAttrInt* underline = pango_attribute_as_int(p);
                switchUnderlineButtons(static_cast<PangoUnderline>(underline->value));
                break;
            }
            case PANGO_ATTR_STRIKETHROUGH: {
                PangoAttrInt* strikethrough = pango_attribute_as_int(p);
                switchStrikethroughButtons(strikethrough->value);
                break;
            }
            case PANGO_ATTR_OVERLINE: {
                PangoAttrInt* overline = pango_attribute_as_int(p);
                switchOverlineButtons(static_cast<PangoOverline>(overline->value));
                break;
            }
            default:
                break;
        }
    }
}

void TextEditorContextMenu::switchStyleButtons(PangoStyle styleValue) {
    this->style = styleValue;
    gtk_toggle_button_set_active(this->tglItalicBtn, false);
    switch (styleValue) {
        case PANGO_STYLE_NORMAL:
            break;
        case PANGO_STYLE_ITALIC:
            gtk_toggle_button_set_active(this->tglItalicBtn, true);
            break;
        case PANGO_STYLE_OBLIQUE:
            break;
        default:
            break;
    }
}

void TextEditorContextMenu::switchWeightButtons(PangoWeight weightValue) {
    this->weight = weightValue;
    gtk_toggle_button_set_active(this->tglBoldBtn, false);
    switch (weightValue) {
        case PANGO_WEIGHT_NORMAL:
            break;
        case PANGO_WEIGHT_BOLD:
            gtk_toggle_button_set_active(this->tglBoldBtn, true);
            break;
        default:
            break;
    }
}

void TextEditorContextMenu::switchUnderlineButtons(PangoUnderline underlineValue) {
    this->underline = underlineValue;
    gtk_toggle_button_set_active(this->tglUnderlineBtn, false);
    switch (underlineValue) {
        case PANGO_UNDERLINE_NONE:
            break;
        case PANGO_UNDERLINE_SINGLE:
            gtk_toggle_button_set_active(this->tglUnderlineBtn, true);
            break;
        default:
            break;
    }
}

void TextEditorContextMenu::switchStrikethroughButtons(int stValue) {
    this->strikethrough = stValue;
    switch (stValue) {
        case TRUE:
            break;
        case FALSE:
            break;
        default:
            break;
    }
}

void TextEditorContextMenu::switchOverlineButtons(PangoOverline overlineValue) {
    this->overline = overlineValue;
    switch (overlineValue) {
        case PANGO_OVERLINE_NONE:
            break;
        case PANGO_OVERLINE_SINGLE:
            break;
        default:
            break;
    }
}
