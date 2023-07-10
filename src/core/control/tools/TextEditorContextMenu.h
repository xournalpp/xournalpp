/*
 * Xournal++
 *
 * Context Menu complementing the text editor
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <list>  // for std::list

#include <gdk/gdk.h>  // for g_signal_connect
#include <gtk/gtk.h>  // for GtkPopover, etc.

class Control;
class TextEditor;
class XojPageView;
enum class TextAlignment;

class TextEditorContextMenu {
public:
    TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView, GtkWidget* xournalWidget);
    ~TextEditorContextMenu();

    void show();
    void hide();

    /**
     * Instance takes ownership of attributes and is responsible for freeing them
     */
    void setAttributes(std::list<PangoAttribute*> attributes);

    void showReducedMenu();
    void showFullMenu();

    void reposition();
    void toggleSecondaryToolbar();

    void changeFont();
    void changeFtColor();
    void changeBgColor();
    void changeAlignment(TextAlignment align);

    void toggleStyle(PangoStyle style);
    void toggleWeight(PangoWeight weight);
    void toggleUnderline(PangoUnderline underline);
    void toggleStrikethrough(int strikethrough);
    void toggleOverline(PangoOverline overline);

    gboolean drawFtColorIcon(GtkWidget* src, cairo_t* cr);
    gboolean drawBgColorIcon(GtkWidget* src, cairo_t* cr);

private:
    void create();
    void applyAttributes();
    void clearAttributes();

    void resetAllButtons();

    void switchStyleButtons(PangoStyle styleValue);
    void switchWeightButtons(PangoWeight weightValue);
    void switchUnderlineButtons(PangoUnderline underlineValue);
    void switchStrikethroughButtons(int stValue);
    void switchOverlineButtons(PangoOverline overlineValue);


private:
    Control* control;
    TextEditor* editor;
    XojPageView* pageView;
    GtkWidget* xournalWidget;

    bool isVisible = false;
    std::list<PangoAttribute*> attributes = {};

    PangoStyle style = PANGO_STYLE_NORMAL;
    PangoWeight weight = PANGO_WEIGHT_NORMAL;
    PangoUnderline underline = PANGO_UNDERLINE_NONE;
    int strikethrough = FALSE;
    PangoOverline overline = PANGO_OVERLINE_NONE;

    /**
     * Default foreground & background colors
     */
    GdkRGBA ftColor{0.0, 0.0, 0.0, 1.0};  // black
    GdkRGBA bgColor{1.0, 0.0, 0.0, 1.0};  // red

    /**
     * UI Elements
     */
    GtkPopover* contextMenu;

    GtkFontButton* fontBtn;

    GtkToggleButton* tglBoldBtn;
    GtkToggleButton* tglItalicBtn;
    GtkToggleButton* tglUnderlineBtn;

    GtkButton* expandTextDecoration;
    GtkImage *arrowDown, *arrowUP;

    GtkButton* ftColorBtn;
    GtkWidget* ftColorIcon;
    GtkButton* bgColorBtn;
    GtkWidget* bgColorIcon;

    GtkToggleButton* alignLeftTgl;
    GtkToggleButton* alignCenterTgl;
    GtkToggleButton* alignRightTgl;

    GtkWidget* textDecoLayout;
    GtkWidget* colorLayout;
    GtkWidget* alignmentLayout;
    GtkWidget* secondaryToolbar;

public:
    static constexpr int COLOR_BAR_HEIGHT = 4;
};
