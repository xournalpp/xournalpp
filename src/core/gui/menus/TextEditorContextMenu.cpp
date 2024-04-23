#include "TextEditorContextMenu.h"

#include <iostream>
#include <string>

#include "control/Control.h"                // for Control
#include "control/tools/TextEditor.h"       // for TextEditor
#include "gui/Builder.h"                    // for Builder
#include "gui/GladeSearchpath.h"            // for GladeSearchPath
#include "gui/PageView.h"                   // for PageView
#include "model/Font.h"                     // for XojFont
#include "model/Text.h"                     // for Text
#include "view/overlays/TextEditionView.h"  // for TextEditionView

static void changeFontInternal(GtkFontButton* src, TextEditorContextMenu* tecm) { tecm->changeFont(); }
static void changeBgColorInternal(GtkColorButton* src, TextEditorContextMenu* tecm) { tecm->changeBgColor(); }
static void toggleAlignLeft(GtkButton* src, TextEditorContextMenu* tecm) { tecm->changeAlignment(TextAlignment::LEFT); }
static void changeFtColorInternal(GtkColorButton* src, TextEditorContextMenu* tecm) { tecm->changeFtColor(); }
static void toggleAlignCenter(GtkButton* src, TextEditorContextMenu* tecm) {
    tecm->changeAlignment(TextAlignment::CENTER);
}
static void toggleAlignRight(GtkButton* src, TextEditorContextMenu* tecm) {
    tecm->changeAlignment(TextAlignment::RIGHT);
}
static gboolean drawFtColorIconInternal(GtkWidget* src, cairo_t* cr, TextEditorContextMenu* tecm) {
    return tecm->drawFtColorIcon(src, cr);
};
static gboolean drawBgColorIconInternal(GtkWidget* src, cairo_t* cr, TextEditorContextMenu* tecm) {
    return tecm->drawBgColorIcon(src, cr);
};
static void tglSecToolbar(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleSecondaryToolbar(); }

static void tglBoldStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleWeight(PANGO_WEIGHT_BOLD); }
static void tglItalicStyle(GtkButton* src, TextEditorContextMenu* tecm) { tecm->toggleStyle(PANGO_STYLE_ITALIC); }
static void tglUnderlineStyle(GtkButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleUnderline(PANGO_UNDERLINE_SINGLE);
}


static void toggleWeightThinClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleWeight(PANGO_WEIGHT_THIN);
}
static void toggleWeightBookClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleWeight(PANGO_WEIGHT_BOOK);
}
static void toggleWeightBoldClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleWeight(PANGO_WEIGHT_BOLD);
}

static void toggleStyleItalicClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleStyle(PANGO_STYLE_ITALIC);
}
static void toggleStyleObliqueClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleStyle(PANGO_STYLE_OBLIQUE);
}
static void toggleUnderlineSingleClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleUnderline(PANGO_UNDERLINE_SINGLE);
}
static void toggleUnderlineSquiggleClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleUnderline(PANGO_UNDERLINE_ERROR);
}
static void toggleUnderlineDoubleClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleUnderline(PANGO_UNDERLINE_DOUBLE);
}
static void toggleStrikethroughClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleStrikethrough(TRUE);
}
static void toggleOverlineSingleClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleOverline(PANGO_OVERLINE_SINGLE);
}

static void toggleSuperScriptClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleScriptRise(5 * PANGO_SCALE);
}
static void toggleSubScriptClb(GtkToggleButton* src, TextEditorContextMenu* tecm) {
    tecm->toggleScriptRise(-(5 * PANGO_SCALE));
}
static void toggleRemoveStyles(GtkButton* btn, TextEditorContextMenu* tecm) { tecm->removeAllAttributes(); }

TextEditorContextMenu::TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView,
                                             GtkWidget* xournalWidget):
        control(control), editor(editor), pageView(pageView), xournalWidget(xournalWidget) {
    // Only for debugging
    std::cout << "TextEditorContextMenu created!" << std::endl;
    this->create();
}

TextEditorContextMenu::~TextEditorContextMenu() {
    gtk_widget_destroy(GTK_WIDGET(this->fontBtn));
    gtk_widget_destroy(GTK_WIDGET(this->tglBoldBtn));
    gtk_widget_destroy(GTK_WIDGET(this->tglItalicBtn));
    gtk_widget_destroy(GTK_WIDGET(this->tglUnderlineBtn));
    gtk_widget_destroy(GTK_WIDGET(this->expandTextDecoration));
    gtk_widget_destroy(GTK_WIDGET(this->ftColorIcon));
    gtk_widget_destroy(GTK_WIDGET(this->ftColorBtn));
    gtk_widget_destroy(GTK_WIDGET(this->bgColorIcon));
    gtk_widget_destroy(GTK_WIDGET(this->bgColorBtn));
    gtk_widget_destroy(GTK_WIDGET(this->alignLeftTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignCenterTgl));
    gtk_widget_destroy(GTK_WIDGET(this->alignRightTgl));
    gtk_widget_destroy(GTK_WIDGET(this->textDecoLayout));
    gtk_widget_destroy(GTK_WIDGET(this->colorLayout));
    gtk_widget_destroy(GTK_WIDGET(this->alignmentLayout));
    gtk_widget_destroy(GTK_WIDGET(this->tglWeightThin));
    gtk_widget_destroy(GTK_WIDGET(this->tglWeightBook));
    gtk_widget_destroy(GTK_WIDGET(this->tglWeightBold));
    gtk_widget_destroy(GTK_WIDGET(this->tglStyleItalic));
    gtk_widget_destroy(GTK_WIDGET(this->tglStyleOblique));
    gtk_widget_destroy(GTK_WIDGET(this->tglUnderlineSingle));
    gtk_widget_destroy(GTK_WIDGET(this->tglUnderlineSquiggle));
    gtk_widget_destroy(GTK_WIDGET(this->tglUnderlineDouble));
    gtk_widget_destroy(GTK_WIDGET(this->tglStrikethrough));
    gtk_widget_destroy(GTK_WIDGET(this->tglOverlineSingle));
    gtk_widget_destroy(GTK_WIDGET(this->tglSuperScript));
    gtk_widget_destroy(GTK_WIDGET(this->tglSubScript));
    gtk_widget_destroy(GTK_WIDGET(this->removeStyles));
    gtk_widget_destroy(GTK_WIDGET(this->secondaryToolbar));
    gtk_popover_set_relative_to(this->contextMenu, NULL);
    std::cout << "TextEditorContextMenu destroyed!" << std::endl;
}

void TextEditorContextMenu::show() {
    if (!isVisible) {
        this->switchAlignmentButtons(this->editor->getTextElement()->getAlignment());
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
        gtk_button_set_image(this->expandTextDecoration, gtk_image_new_from_icon_name("go-down", GTK_ICON_SIZE_BUTTON));
        gtk_widget_set_visible(this->secondaryToolbar, false);
    } else {
        std::cout << "Hide secondary toolbar!" << std::endl;
        gtk_button_set_image(this->expandTextDecoration, gtk_image_new_from_icon_name("go-up", GTK_ICON_SIZE_BUTTON));
        gtk_widget_set_visible(this->secondaryToolbar, true);
    }
}

void TextEditorContextMenu::create() {

    Builder builder(this->control->getGladeSearchPath(), "textEditorContextMenu.glade");

    this->contextMenu = GTK_POPOVER(builder.get("textEditorContextMenu"));
    gtk_popover_set_relative_to(this->contextMenu, this->xournalWidget);
    gtk_popover_set_constrain_to(this->contextMenu, GTK_POPOVER_CONSTRAINT_WINDOW);
    gtk_popover_set_modal(this->contextMenu, false);
    gtk_widget_set_can_focus(GTK_WIDGET(this->contextMenu), false);
    gtk_widget_hide(GTK_WIDGET(this->contextMenu));

    this->fontBtn = GTK_FONT_BUTTON(builder.get("btnFontChooser"));
    g_signal_connect(this->fontBtn, "font-set", G_CALLBACK(changeFontInternal), this);


    this->tglBoldBtn = GTK_TOGGLE_BUTTON(builder.get("btnDecoBold"));
    this->tglItalicBtn = GTK_TOGGLE_BUTTON(builder.get("btnDecoItalic"));
    this->tglUnderlineBtn = GTK_TOGGLE_BUTTON(builder.get("btnDecoUnderline"));
    g_signal_connect(tglBoldBtn, "released", G_CALLBACK(tglBoldStyle), this);
    g_signal_connect(tglItalicBtn, "released", G_CALLBACK(tglItalicStyle), this);
    g_signal_connect(tglUnderlineBtn, "released", G_CALLBACK(tglUnderlineStyle), this);

    this->expandTextDecoration = GTK_BUTTON(builder.get("btnDecoExpand"));
    gtk_button_set_image(this->expandTextDecoration, gtk_image_new_from_icon_name("go-down", GTK_ICON_SIZE_BUTTON));
    g_signal_connect(expandTextDecoration, "clicked", G_CALLBACK(tglSecToolbar), this);


    this->ftColorBtn = GTK_BUTTON(builder.get("btnFontColor"));
    this->bgColorBtn = GTK_BUTTON(builder.get("btnBgColor"));
    g_signal_connect(this->ftColorBtn, "clicked", G_CALLBACK(changeFtColorInternal), this);
    g_signal_connect(this->bgColorBtn, "clicked", G_CALLBACK(changeBgColorInternal), this);

    this->ftColorIcon = GTK_WIDGET(builder.get("imgFtColor"));
    g_signal_connect(this->ftColorIcon, "draw", G_CALLBACK(drawFtColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->ftColorBtn), this->ftColorIcon);

    this->bgColorIcon = GTK_WIDGET(builder.get("imgBgColor"));
    g_signal_connect(this->bgColorIcon, "draw", G_CALLBACK(drawBgColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->bgColorBtn), this->bgColorIcon);

    this->alignLeftTgl = GTK_TOGGLE_BUTTON(builder.get("btnAlignLeft"));
    this->alignCenterTgl = GTK_TOGGLE_BUTTON(builder.get("btnAlignCenter"));
    this->alignRightTgl = GTK_TOGGLE_BUTTON(builder.get("btnAlignRight"));
    g_signal_connect(this->alignLeftTgl, "released", G_CALLBACK(toggleAlignLeft), this);
    g_signal_connect(this->alignCenterTgl, "released", G_CALLBACK(toggleAlignCenter), this);
    g_signal_connect(this->alignRightTgl, "released", G_CALLBACK(toggleAlignRight), this);

    this->textDecoLayout = GTK_WIDGET(builder.get("textDecoLayout"));
    this->colorLayout = GTK_WIDGET(builder.get("colorLayout"));
    this->alignmentLayout = GTK_WIDGET(builder.get("alignmentLayout"));
    this->secondaryToolbar = GTK_WIDGET(builder.get("secondaryToolbar"));


    tglWeightThin = GTK_TOGGLE_BUTTON(builder.get("btnWeightThin"));
    tglWeightBook = GTK_TOGGLE_BUTTON(builder.get("btnWeightBook"));
    tglWeightBold = GTK_TOGGLE_BUTTON(builder.get("btnWeightBold"));
    g_signal_connect(this->tglWeightThin, "released", G_CALLBACK(toggleWeightThinClb), this);
    g_signal_connect(this->tglWeightBook, "released", G_CALLBACK(toggleWeightBookClb), this);
    g_signal_connect(this->tglWeightBold, "released", G_CALLBACK(toggleWeightBoldClb), this);

    tglStyleItalic = GTK_TOGGLE_BUTTON(builder.get("btnStyleItalic"));
    tglStyleOblique = GTK_TOGGLE_BUTTON(builder.get("btnStyleOblique"));
    g_signal_connect(this->tglStyleItalic, "released", G_CALLBACK(toggleStyleItalicClb), this);
    g_signal_connect(this->tglStyleOblique, "released", G_CALLBACK(toggleStyleObliqueClb), this);

    tglUnderlineSingle = GTK_TOGGLE_BUTTON(builder.get("btnUnderlineSingle"));
    tglUnderlineSquiggle = GTK_TOGGLE_BUTTON(builder.get("btnUnderlineError"));
    tglUnderlineDouble = GTK_TOGGLE_BUTTON(builder.get("btnUnderlineDouble"));
    tglStrikethrough = GTK_TOGGLE_BUTTON(builder.get("btnStrikethrough"));
    tglOverlineSingle = GTK_TOGGLE_BUTTON(builder.get("btnOverlineSingle"));
    g_signal_connect(this->tglUnderlineSingle, "released", G_CALLBACK(toggleUnderlineSingleClb), this);
    g_signal_connect(this->tglUnderlineSquiggle, "released", G_CALLBACK(toggleUnderlineSquiggleClb), this);
    g_signal_connect(this->tglUnderlineDouble, "released", G_CALLBACK(toggleUnderlineDoubleClb), this);
    g_signal_connect(this->tglStrikethrough, "released", G_CALLBACK(toggleStrikethroughClb), this);
    g_signal_connect(this->tglOverlineSingle, "released", G_CALLBACK(toggleOverlineSingleClb), this);

    tglSuperScript = GTK_TOGGLE_BUTTON(builder.get("btnSuperscript"));
    tglSubScript = GTK_TOGGLE_BUTTON(builder.get("btnSubscript"));
    g_signal_connect(this->tglSuperScript, "released", G_CALLBACK(toggleSuperScriptClb), this);
    g_signal_connect(this->tglSubScript, "released", G_CALLBACK(toggleSubScriptClb), this);

    removeStyles = GTK_BUTTON(builder.get("btnRemoveStyle"));
    g_signal_connect(this->removeStyles, "clicked", G_CALLBACK(toggleRemoveStyles), this);
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
    this->switchAlignmentButtons(align);
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
    std::cout << "Weight" << std::endl;
    if (this->weight == weight) {
        this->editor->addTextAttributeInline(pango_attr_weight_new(PANGO_WEIGHT_NORMAL));
        this->switchWeightButtons(PANGO_WEIGHT_NORMAL);
    } else {
        this->editor->addTextAttributeInline(pango_attr_weight_new(weight));
        this->switchWeightButtons(weight);
    }
}

void TextEditorContextMenu::toggleStyle(PangoStyle style) {
    std::cout << "Style" << std::endl;
    if (this->style == style) {
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

void TextEditorContextMenu::toggleStrikethrough(int strikethrough) {
    std::cout << "Stikethrough" << std::endl;
    if (this->strikethrough == TRUE) {
        this->editor->addTextAttributeInline(pango_attr_strikethrough_new(FALSE));
        this->switchStrikethroughButtons(FALSE);
    } else {
        this->editor->addTextAttributeInline(pango_attr_strikethrough_new(TRUE));
        this->switchStrikethroughButtons(TRUE);
    }
}

void TextEditorContextMenu::toggleOverline(PangoOverline overline) {
    std::cout << "Overline" << std::endl;
    if (this->overline == overline) {
        this->editor->addTextAttributeInline(pango_attr_overline_new(PANGO_OVERLINE_NONE));
        this->switchOverlineButtons(PANGO_OVERLINE_NONE);
    } else {
        this->editor->addTextAttributeInline(pango_attr_overline_new(overline));
        this->switchOverlineButtons(overline);
    }
}

void TextEditorContextMenu::toggleScriptRise(int rise) {
    std::cout << "Script rise: " << rise << std::endl;
    if (this->rise == rise) {
        std::cout << "Got here 1!" << std::endl;
        this->editor->addTextAttributeInline(pango_attr_rise_new(0));
        this->switchRiseButtons(0);
    } else {
        std::cout << "Got here 2!" << std::endl;
        this->editor->addTextAttributeInline(pango_attr_rise_new(rise));
        this->switchRiseButtons(rise);
    }
}

void TextEditorContextMenu::setAttributes(std::list<PangoAttribute*> attributes) {
    std::cout << "ContectMenu.setAttributes: " << attributes.size() << std::endl;
    this->clearAttributes();
    this->resetContextMenuState();
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
                this->bgColor.blue = double(bgColor->color.blue) / double(UINT16_MAX);
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
            case PANGO_ATTR_RISE: {
                PangoAttrInt* rise = pango_attribute_as_int(p);
                switchRiseButtons(rise->value);
            }
            default:
                break;
        }
    }
}

void TextEditorContextMenu::switchStyleButtons(PangoStyle styleValue) {
    this->style = styleValue;
    switch (styleValue) {
        case PANGO_STYLE_ITALIC:
            gtk_toggle_button_set_active(this->tglItalicBtn, true);
            gtk_toggle_button_set_active(this->tglStyleItalic, true);
            gtk_toggle_button_set_active(this->tglStyleOblique, false);
            break;
        case PANGO_STYLE_OBLIQUE:
            gtk_toggle_button_set_active(this->tglItalicBtn, false);
            gtk_toggle_button_set_active(this->tglStyleItalic, false);
            gtk_toggle_button_set_active(this->tglStyleOblique, true);
            break;
        case PANGO_STYLE_NORMAL:
        default:
            gtk_toggle_button_set_active(this->tglItalicBtn, false);
            gtk_toggle_button_set_active(this->tglStyleItalic, false);
            gtk_toggle_button_set_active(this->tglStyleOblique, false);
            break;
    }
}

void TextEditorContextMenu::switchWeightButtons(PangoWeight weightValue) {
    this->weight = weightValue;
    switch (weightValue) {
        case PANGO_WEIGHT_THIN:
            gtk_toggle_button_set_active(this->tglBoldBtn, false);
            gtk_toggle_button_set_active(this->tglWeightThin, true);
            gtk_toggle_button_set_active(this->tglWeightBook, false);
            gtk_toggle_button_set_active(this->tglWeightBold, false);
            break;
        case PANGO_WEIGHT_BOOK:
            gtk_toggle_button_set_active(this->tglBoldBtn, false);
            gtk_toggle_button_set_active(this->tglWeightThin, false);
            gtk_toggle_button_set_active(this->tglWeightBook, true);
            gtk_toggle_button_set_active(this->tglWeightBold, false);
            break;
        case PANGO_WEIGHT_BOLD:
            gtk_toggle_button_set_active(this->tglBoldBtn, true);
            gtk_toggle_button_set_active(this->tglWeightThin, false);
            gtk_toggle_button_set_active(this->tglWeightBook, false);
            gtk_toggle_button_set_active(this->tglWeightBold, true);
            break;
        case PANGO_WEIGHT_NORMAL:
        default:
            gtk_toggle_button_set_active(this->tglBoldBtn, false);
            gtk_toggle_button_set_active(this->tglWeightThin, false);
            gtk_toggle_button_set_active(this->tglWeightBook, false);
            gtk_toggle_button_set_active(this->tglWeightBold, false);
            break;
    }
}

void TextEditorContextMenu::switchUnderlineButtons(PangoUnderline underlineValue) {
    this->underline = underlineValue;
    switch (underlineValue) {
        case PANGO_UNDERLINE_SINGLE:
            gtk_toggle_button_set_active(this->tglUnderlineBtn, true);
            gtk_toggle_button_set_active(this->tglUnderlineSingle, true);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle, false);
            gtk_toggle_button_set_active(this->tglUnderlineDouble, false);
            break;
        case PANGO_UNDERLINE_ERROR:
            gtk_toggle_button_set_active(this->tglUnderlineBtn, false);
            gtk_toggle_button_set_active(this->tglUnderlineSingle, false);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle, true);
            gtk_toggle_button_set_active(this->tglUnderlineDouble, false);
            break;
        case PANGO_UNDERLINE_DOUBLE:
            gtk_toggle_button_set_active(this->tglUnderlineBtn, false);
            gtk_toggle_button_set_active(this->tglUnderlineSingle, false);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle, false);
            gtk_toggle_button_set_active(this->tglUnderlineDouble, true);
            break;
        case PANGO_UNDERLINE_NONE:
        default:
            gtk_toggle_button_set_active(this->tglUnderlineBtn, false);
            gtk_toggle_button_set_active(this->tglUnderlineSingle, false);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle, false);
            gtk_toggle_button_set_active(this->tglUnderlineDouble, false);
            break;
    }
}

void TextEditorContextMenu::switchStrikethroughButtons(int stValue) {
    this->strikethrough = stValue;
    switch (stValue) {
        case TRUE:
            gtk_toggle_button_set_active(this->tglStrikethrough, true);
            break;
        case FALSE:
            gtk_toggle_button_set_active(this->tglStrikethrough, false);
            break;
        default:
            gtk_toggle_button_set_active(this->tglStrikethrough, false);
            break;
    }
}

void TextEditorContextMenu::switchOverlineButtons(PangoOverline overlineValue) {
    this->overline = overlineValue;
    switch (overlineValue) {
        case PANGO_OVERLINE_SINGLE:
            gtk_toggle_button_set_active(this->tglOverlineSingle, true);
            break;
        case PANGO_OVERLINE_NONE:
        default:
            gtk_toggle_button_set_active(this->tglOverlineSingle, false);
            break;
    }
}

void TextEditorContextMenu::resetContextMenuState() {
    PangoFontDescription* desc = pango_font_description_from_string(editor->getTextElement()->getFontName().c_str());
    pango_font_description_set_size(desc, editor->getTextElement()->getFontSize() * PANGO_SCALE);
    gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(this->fontBtn), desc);
    ftColor = {0.0, 0.0, 0.0, 1.0};
    bgColor = {1.0, 1.0, 1.0, 0.0};
    gtk_widget_queue_draw(GTK_WIDGET(this->ftColorBtn));
    gtk_widget_queue_draw(GTK_WIDGET(this->bgColorBtn));
    this->switchWeightButtons(PANGO_WEIGHT_NORMAL);
    this->switchStyleButtons(PANGO_STYLE_NORMAL);
    this->switchUnderlineButtons(PANGO_UNDERLINE_NONE);
    this->switchStrikethroughButtons(FALSE);
    this->switchOverlineButtons(PANGO_OVERLINE_NONE);
}

void TextEditorContextMenu::switchAlignmentButtons(TextAlignment alignment) {
    switch (alignment) {
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
}

void TextEditorContextMenu::switchRiseButtons(int riseValue) {
    this->rise = riseValue;
    if (riseValue > 0) {
        gtk_toggle_button_set_active(this->tglSuperScript, true);
        gtk_toggle_button_set_active(this->tglSubScript, false);
    } else if (riseValue < 0) {
        gtk_toggle_button_set_active(this->tglSuperScript, false);
        gtk_toggle_button_set_active(this->tglSubScript, true);
    } else {
        gtk_toggle_button_set_active(this->tglSuperScript, false);
        gtk_toggle_button_set_active(this->tglSubScript, false);
    }
}

void TextEditorContextMenu::removeAllAttributes() {
    std::cout << "Remove all attributes" << std::endl;
    this->editor->clearAttributes();
    this->clearAttributes();
    this->resetContextMenuState();
}
