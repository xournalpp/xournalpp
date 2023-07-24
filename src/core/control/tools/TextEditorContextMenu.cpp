#include "TextEditorContextMenu.h"

#include <iostream>
#include <string>

#include "control/Control.h"                // for Control
#include "gui/GladeSearchpath.h"            // for GladeSearchPath
#include "gui/PageView.h"                   // for PageView
#include "model/Font.h"                     // for XojFont
#include "model/Text.h"                     // for Text
#include "view/overlays/TextEditionView.h"  // for TextEditionView

#include "TextEditor.h"  // for TextEditor

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

TextEditorContextMenu::~TextEditorContextMenu() { std::cout << "TextEditorContextMenu destroyed!" << std::endl; }

void TextEditorContextMenu::show() {
    if (!isVisible) {
        this->switchAlignmentButtons(this->editor->getTextElement()->getAlignment());
        this->reposition();
        this->showReducedMenu();
        gtk_popover_popup(this->contextMenu.get());
        isVisible = true;
        std::cout << "Popup menu should be shown" << std::endl;
    }
}

void TextEditorContextMenu::hide() {
    if (isVisible) {
        gtk_popover_popdown(this->contextMenu.get());
        isVisible = false;
        std::cout << "Popup menu should be hidden" << std::endl;
    }
}

void TextEditorContextMenu::showReducedMenu() {
    gtk_widget_set_visible(GTK_WIDGET(this->fontBtn.get()), false);
    gtk_widget_set_visible(this->textDecoLayout.get(), false);
    gtk_widget_set_visible(this->colorLayout.get(), false);
    gtk_widget_set_visible(this->alignmentLayout.get(), true);
    gtk_widget_set_visible(this->secondaryToolbar.get(), false);
}

void TextEditorContextMenu::showFullMenu() {
    gtk_widget_set_visible(GTK_WIDGET(this->fontBtn.get()), true);
    gtk_widget_set_visible(this->textDecoLayout.get(), true);
    gtk_widget_set_visible(this->colorLayout.get(), true);
    gtk_widget_set_visible(this->alignmentLayout.get(), true);
    gtk_widget_set_visible(this->secondaryToolbar.get(), false);
}

void TextEditorContextMenu::reposition() {
    int padding = xoj::view::TextEditionView::PADDING_IN_PIXELS;
    Range r = this->editor->getContentBoundingBox();
    GdkRectangle rect{this->pageView->getX() + int(r.getX() * this->pageView->getZoom()),
                      this->pageView->getY() + int(r.getY() * this->pageView->getZoom()) - padding,
                      int(r.getWidth() * this->pageView->getZoom()), int(r.getHeight() * this->pageView->getZoom())};
    gtk_popover_set_pointing_to(this->contextMenu.get(), &rect);
}

void TextEditorContextMenu::toggleSecondaryToolbar() {
    if (gtk_widget_is_visible(this->secondaryToolbar.get())) {
        std::cout << "Show secondary toolbar!" << std::endl;
        gtk_button_set_image(this->expandTextDecoration.get(),
                             gtk_image_new_from_icon_name("go-down", GTK_ICON_SIZE_BUTTON));
        gtk_widget_set_visible(this->secondaryToolbar.get(), false);
    } else {
        std::cout << "Hide secondary toolbar!" << std::endl;
        gtk_button_set_image(this->expandTextDecoration.get(),
                             gtk_image_new_from_icon_name("go-up", GTK_ICON_SIZE_BUTTON));
        gtk_widget_set_visible(this->secondaryToolbar.get(), true);
    }
}

void TextEditorContextMenu::create() {
    auto filepath = this->control->getGladeSearchPath()->findFile("", "textEditorContextMenu.glade");

    GtkBuilder* builder = gtk_builder_new();

    GError* err = NULL;
    if (gtk_builder_add_from_file(builder, filepath.u8string().c_str(), &err) == 0) {
        std::cout << err->message << std::endl;
    }

    this->contextMenu = xoj::util::GObjectSPtr<GtkPopover>(
            GTK_POPOVER(gtk_builder_get_object(builder, "textEditorContextMenu")), xoj::util::adopt);
    gtk_popover_set_relative_to(this->contextMenu.get(), this->xournalWidget);
    gtk_popover_set_constrain_to(this->contextMenu.get(), GTK_POPOVER_CONSTRAINT_WINDOW);
    gtk_popover_set_modal(this->contextMenu.get(), false);
    gtk_widget_set_can_focus(GTK_WIDGET(this->contextMenu.get()), false);
    gtk_widget_hide(GTK_WIDGET(this->contextMenu.get()));

    this->fontBtn = xoj::util::GObjectSPtr<GtkFontButton>(
            GTK_FONT_BUTTON(gtk_builder_get_object(builder, "btnFontChooser")), xoj::util::adopt);
    g_signal_connect(this->fontBtn.get(), "font-set", G_CALLBACK(changeFontInternal), this);


    this->tglBoldBtn = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnDecoBold")), xoj::util::adopt);
    this->tglItalicBtn = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnDecoItalic")), xoj::util::adopt);
    this->tglUnderlineBtn = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnDecoUnderline")), xoj::util::adopt);
    g_signal_connect(tglBoldBtn.get(), "released", G_CALLBACK(tglBoldStyle), this);
    g_signal_connect(tglItalicBtn.get(), "released", G_CALLBACK(tglItalicStyle), this);
    g_signal_connect(tglUnderlineBtn.get(), "released", G_CALLBACK(tglUnderlineStyle), this);

    this->expandTextDecoration = xoj::util::GObjectSPtr<GtkButton>(
            GTK_BUTTON(gtk_builder_get_object(builder, "btnDecoExpand")), xoj::util::adopt);
    gtk_button_set_image(this->expandTextDecoration.get(),
                         gtk_image_new_from_icon_name("go-down", GTK_ICON_SIZE_BUTTON));
    g_signal_connect(expandTextDecoration.get(), "clicked", G_CALLBACK(tglSecToolbar), this);


    this->ftColorBtn = xoj::util::GObjectSPtr<GtkButton>(GTK_BUTTON(gtk_builder_get_object(builder, "btnFontColor")),
                                                         xoj::util::adopt);
    this->bgColorBtn = xoj::util::GObjectSPtr<GtkButton>(GTK_BUTTON(gtk_builder_get_object(builder, "btnBgColor")),
                                                         xoj::util::adopt);
    g_signal_connect(this->ftColorBtn.get(), "clicked", G_CALLBACK(changeFtColorInternal), this);
    g_signal_connect(this->bgColorBtn.get(), "clicked", G_CALLBACK(changeBgColorInternal), this);

    this->ftColorIcon = xoj::util::GObjectSPtr<GtkWidget>(GTK_WIDGET(gtk_builder_get_object(builder, "imgFtColor")),
                                                          xoj::util::adopt);
    g_signal_connect(this->ftColorIcon.get(), "draw", G_CALLBACK(drawFtColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->ftColorBtn.get()), this->ftColorIcon.get());

    this->bgColorIcon = xoj::util::GObjectSPtr<GtkWidget>(GTK_WIDGET(gtk_builder_get_object(builder, "imgBgColor")),
                                                          xoj::util::adopt);
    g_signal_connect(this->bgColorIcon.get(), "draw", G_CALLBACK(drawBgColorIconInternal), this);
    gtk_button_set_image(GTK_BUTTON(this->bgColorBtn.get()), this->bgColorIcon.get());

    this->alignLeftTgl = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignLeft")), xoj::util::adopt);
    this->alignCenterTgl = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignCenter")), xoj::util::adopt);
    this->alignRightTgl = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnAlignRight")), xoj::util::adopt);
    g_signal_connect(this->alignLeftTgl.get(), "released", G_CALLBACK(toggleAlignLeft), this);
    g_signal_connect(this->alignCenterTgl.get(), "released", G_CALLBACK(toggleAlignCenter), this);
    g_signal_connect(this->alignRightTgl.get(), "released", G_CALLBACK(toggleAlignRight), this);

    this->textDecoLayout = xoj::util::GObjectSPtr<GtkWidget>(
            GTK_WIDGET(gtk_builder_get_object(builder, "textDecoLayout")), xoj::util::adopt);
    this->colorLayout = xoj::util::GObjectSPtr<GtkWidget>(GTK_WIDGET(gtk_builder_get_object(builder, "colorLayout")),
                                                          xoj::util::adopt);
    this->alignmentLayout = xoj::util::GObjectSPtr<GtkWidget>(
            GTK_WIDGET(gtk_builder_get_object(builder, "alignmentLayout")), xoj::util::adopt);
    this->secondaryToolbar = xoj::util::GObjectSPtr<GtkWidget>(
            GTK_WIDGET(gtk_builder_get_object(builder, "secondaryToolbar")), xoj::util::adopt);


    tglWeightThin = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnWeightThin")), xoj::util::adopt);
    tglWeightBook = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnWeightBook")), xoj::util::adopt);
    tglWeightBold = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnWeightBold")), xoj::util::adopt);
    g_signal_connect(this->tglWeightThin.get(), "released", G_CALLBACK(toggleWeightThinClb), this);
    g_signal_connect(this->tglWeightBook.get(), "released", G_CALLBACK(toggleWeightBookClb), this);
    g_signal_connect(this->tglWeightBold.get(), "released", G_CALLBACK(toggleWeightBoldClb), this);

    tglStyleItalic = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnStyleItalic")), xoj::util::adopt);
    tglStyleOblique = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnStyleOblique")), xoj::util::adopt);
    g_signal_connect(this->tglStyleItalic.get(), "released", G_CALLBACK(toggleStyleItalicClb), this);
    g_signal_connect(this->tglStyleOblique.get(), "released", G_CALLBACK(toggleStyleObliqueClb), this);

    tglUnderlineSingle = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnUnderlineSingle")), xoj::util::adopt);
    tglUnderlineSquiggle = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnUnderlineError")), xoj::util::adopt);
    tglUnderlineDouble = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnUnderlineDouble")), xoj::util::adopt);
    tglStrikethrough = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnStrikethrough")), xoj::util::adopt);
    tglOverlineSingle = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnOverlineSingle")), xoj::util::adopt);
    g_signal_connect(this->tglUnderlineSingle.get(), "released", G_CALLBACK(toggleUnderlineSingleClb), this);
    g_signal_connect(this->tglUnderlineSquiggle.get(), "released", G_CALLBACK(toggleUnderlineSquiggleClb), this);
    g_signal_connect(this->tglUnderlineDouble.get(), "released", G_CALLBACK(toggleUnderlineDoubleClb), this);
    g_signal_connect(this->tglStrikethrough.get(), "released", G_CALLBACK(toggleStrikethroughClb), this);
    g_signal_connect(this->tglOverlineSingle.get(), "released", G_CALLBACK(toggleOverlineSingleClb), this);

    tglSuperScript = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnSuperscript")), xoj::util::adopt);
    tglSubScript = xoj::util::GObjectSPtr<GtkToggleButton>(
            GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "btnSubscript")), xoj::util::adopt);
    g_signal_connect(this->tglSuperScript.get(), "released", G_CALLBACK(toggleSuperScriptClb), this);
    g_signal_connect(this->tglSubScript.get(), "released", G_CALLBACK(toggleSubScriptClb), this);

    removeStyles = xoj::util::GObjectSPtr<GtkButton>(GTK_BUTTON(gtk_builder_get_object(builder, "btnRemoveStyle")),
                                                     xoj::util::adopt);
    g_signal_connect(this->removeStyles.get(), "clicked", G_CALLBACK(toggleRemoveStyles), this);

    g_object_unref(G_OBJECT(builder));
}

void TextEditorContextMenu::changeFont() {
    PangoFontDescription* desc = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(this->fontBtn.get()));
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
                gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(this->fontBtn.get()), desc->desc);
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
            gtk_toggle_button_set_active(this->tglItalicBtn.get(), true);
            gtk_toggle_button_set_active(this->tglStyleItalic.get(), true);
            gtk_toggle_button_set_active(this->tglStyleOblique.get(), false);
            break;
        case PANGO_STYLE_OBLIQUE:
            gtk_toggle_button_set_active(this->tglItalicBtn.get(), false);
            gtk_toggle_button_set_active(this->tglStyleItalic.get(), false);
            gtk_toggle_button_set_active(this->tglStyleOblique.get(), true);
            break;
        case PANGO_STYLE_NORMAL:
        default:
            gtk_toggle_button_set_active(this->tglItalicBtn.get(), false);
            gtk_toggle_button_set_active(this->tglStyleItalic.get(), false);
            gtk_toggle_button_set_active(this->tglStyleOblique.get(), false);
            break;
    }
}

void TextEditorContextMenu::switchWeightButtons(PangoWeight weightValue) {
    this->weight = weightValue;
    switch (weightValue) {
        case PANGO_WEIGHT_THIN:
            gtk_toggle_button_set_active(this->tglBoldBtn.get(), false);
            gtk_toggle_button_set_active(this->tglWeightThin.get(), true);
            gtk_toggle_button_set_active(this->tglWeightBook.get(), false);
            gtk_toggle_button_set_active(this->tglWeightBold.get(), false);
            break;
        case PANGO_WEIGHT_BOOK:
            gtk_toggle_button_set_active(this->tglBoldBtn.get(), false);
            gtk_toggle_button_set_active(this->tglWeightThin.get(), false);
            gtk_toggle_button_set_active(this->tglWeightBook.get(), true);
            gtk_toggle_button_set_active(this->tglWeightBold.get(), false);
            break;
        case PANGO_WEIGHT_BOLD:
            gtk_toggle_button_set_active(this->tglBoldBtn.get(), true);
            gtk_toggle_button_set_active(this->tglWeightThin.get(), false);
            gtk_toggle_button_set_active(this->tglWeightBook.get(), false);
            gtk_toggle_button_set_active(this->tglWeightBold.get(), true);
            break;
        case PANGO_WEIGHT_NORMAL:
        default:
            gtk_toggle_button_set_active(this->tglBoldBtn.get(), false);
            gtk_toggle_button_set_active(this->tglWeightThin.get(), false);
            gtk_toggle_button_set_active(this->tglWeightBook.get(), false);
            gtk_toggle_button_set_active(this->tglWeightBold.get(), false);
            break;
    }
}

void TextEditorContextMenu::switchUnderlineButtons(PangoUnderline underlineValue) {
    this->underline = underlineValue;
    switch (underlineValue) {
        case PANGO_UNDERLINE_SINGLE:
            gtk_toggle_button_set_active(this->tglUnderlineBtn.get(), true);
            gtk_toggle_button_set_active(this->tglUnderlineSingle.get(), true);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineDouble.get(), false);
            break;
        case PANGO_UNDERLINE_ERROR:
            gtk_toggle_button_set_active(this->tglUnderlineBtn.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineSingle.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle.get(), true);
            gtk_toggle_button_set_active(this->tglUnderlineDouble.get(), false);
            break;
        case PANGO_UNDERLINE_DOUBLE:
            gtk_toggle_button_set_active(this->tglUnderlineBtn.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineSingle.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineDouble.get(), true);
            break;
        case PANGO_UNDERLINE_NONE:
        default:
            gtk_toggle_button_set_active(this->tglUnderlineBtn.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineSingle.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineSquiggle.get(), false);
            gtk_toggle_button_set_active(this->tglUnderlineDouble.get(), false);
            break;
    }
}

void TextEditorContextMenu::switchStrikethroughButtons(int stValue) {
    this->strikethrough = stValue;
    switch (stValue) {
        case TRUE:
            gtk_toggle_button_set_active(this->tglStrikethrough.get(), true);
            break;
        case FALSE:
            gtk_toggle_button_set_active(this->tglStrikethrough.get(), false);
            break;
        default:
            gtk_toggle_button_set_active(this->tglStrikethrough.get(), false);
            break;
    }
}

void TextEditorContextMenu::switchOverlineButtons(PangoOverline overlineValue) {
    this->overline = overlineValue;
    switch (overlineValue) {
        case PANGO_OVERLINE_SINGLE:
            gtk_toggle_button_set_active(this->tglOverlineSingle.get(), true);
            break;
        case PANGO_OVERLINE_NONE:
        default:
            gtk_toggle_button_set_active(this->tglOverlineSingle.get(), false);
            break;
    }
}

void TextEditorContextMenu::resetContextMenuState() {
    PangoFontDescription* desc = pango_font_description_from_string(editor->getTextElement()->getFontName().c_str());
    pango_font_description_set_size(desc, editor->getTextElement()->getFontSize() * PANGO_SCALE);
    gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(this->fontBtn.get()), desc);
    ftColor = {0.0, 0.0, 0.0, 1.0};
    bgColor = {1.0, 1.0, 1.0, 0.0};
    gtk_widget_queue_draw(GTK_WIDGET(this->ftColorBtn.get()));
    gtk_widget_queue_draw(GTK_WIDGET(this->bgColorBtn.get()));
    this->switchWeightButtons(PANGO_WEIGHT_NORMAL);
    this->switchStyleButtons(PANGO_STYLE_NORMAL);
    this->switchUnderlineButtons(PANGO_UNDERLINE_NONE);
    this->switchStrikethroughButtons(FALSE);
    this->switchOverlineButtons(PANGO_OVERLINE_NONE);
}

void TextEditorContextMenu::switchAlignmentButtons(TextAlignment alignment) {
    switch (alignment) {
        case TextAlignment::LEFT:
            gtk_toggle_button_set_active(this->alignLeftTgl.get(), true);
            gtk_toggle_button_set_active(this->alignCenterTgl.get(), false);
            gtk_toggle_button_set_active(this->alignRightTgl.get(), false);
            break;
        case TextAlignment::CENTER:
            gtk_toggle_button_set_active(this->alignLeftTgl.get(), false);
            gtk_toggle_button_set_active(this->alignCenterTgl.get(), true);
            gtk_toggle_button_set_active(this->alignRightTgl.get(), false);
            break;
        case TextAlignment::RIGHT:
            gtk_toggle_button_set_active(this->alignLeftTgl.get(), false);
            gtk_toggle_button_set_active(this->alignCenterTgl.get(), false);
            gtk_toggle_button_set_active(this->alignRightTgl.get(), true);
            break;
        default:
            gtk_toggle_button_set_active(this->alignLeftTgl.get(), false);
            gtk_toggle_button_set_active(this->alignCenterTgl.get(), false);
            gtk_toggle_button_set_active(this->alignRightTgl.get(), false);
            break;
    }
}

void TextEditorContextMenu::switchRiseButtons(int riseValue) {
    this->rise = riseValue;
    if (riseValue > 0) {
        gtk_toggle_button_set_active(this->tglSuperScript.get(), true);
        gtk_toggle_button_set_active(this->tglSubScript.get(), false);
    } else if (riseValue < 0) {
        gtk_toggle_button_set_active(this->tglSuperScript.get(), false);
        gtk_toggle_button_set_active(this->tglSubScript.get(), true);
    } else {
        gtk_toggle_button_set_active(this->tglSuperScript.get(), false);
        gtk_toggle_button_set_active(this->tglSubScript.get(), false);
    }
}

void TextEditorContextMenu::removeAllAttributes() {
    std::cout << "Remove all attributes" << std::endl;
    this->editor->clearAttributes();
    this->clearAttributes();
    this->resetContextMenuState();
}
