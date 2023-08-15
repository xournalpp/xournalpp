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

#include "util/raii/GObjectSPtr.h"  // for xoj::util::GObjectSPtr

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
    void toggleScriptRise(int rise);

    void removeAllAttributes();

    gboolean drawFtColorIcon(GtkWidget* src, cairo_t* cr);
    gboolean drawBgColorIcon(GtkWidget* src, cairo_t* cr);

private:
    void create();
    void applyAttributes();
    void clearAttributes();

    void resetContextMenuState();

    void switchAlignmentButtons(TextAlignment alignment);

    void switchStyleButtons(PangoStyle styleValue);
    void switchWeightButtons(PangoWeight weightValue);
    void switchUnderlineButtons(PangoUnderline underlineValue);
    void switchStrikethroughButtons(int stValue);
    void switchOverlineButtons(PangoOverline overlineValue);
    void switchRiseButtons(int riseValue);


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
    int rise = 0;

    /**
     * Default foreground & background colors
     */
    GdkRGBA ftColor{0.0, 0.0, 0.0, 1.0};  // black
    GdkRGBA bgColor{1.0, 1.0, 1.0, 0.0};  // transparent white

    /**
     * UI Elements
     */
    GtkPopover* contextMenu;

    xoj::util::GObjectSPtr<GtkFontButton> fontBtn;

    xoj::util::GObjectSPtr<GtkToggleButton> tglBoldBtn;
    xoj::util::GObjectSPtr<GtkToggleButton> tglItalicBtn;
    xoj::util::GObjectSPtr<GtkToggleButton> tglUnderlineBtn;

    xoj::util::GObjectSPtr<GtkButton> expandTextDecoration;

    xoj::util::GObjectSPtr<GtkButton> ftColorBtn;
    xoj::util::GObjectSPtr<GtkWidget> ftColorIcon;
    xoj::util::GObjectSPtr<GtkButton> bgColorBtn;
    xoj::util::GObjectSPtr<GtkWidget> bgColorIcon;

    xoj::util::GObjectSPtr<GtkToggleButton> alignLeftTgl;
    xoj::util::GObjectSPtr<GtkToggleButton> alignCenterTgl;
    xoj::util::GObjectSPtr<GtkToggleButton> alignRightTgl;

    xoj::util::GObjectSPtr<GtkWidget> textDecoLayout;
    xoj::util::GObjectSPtr<GtkWidget> colorLayout;
    xoj::util::GObjectSPtr<GtkWidget> alignmentLayout;
    xoj::util::GObjectSPtr<GtkWidget> secondaryToolbar;

    xoj::util::GObjectSPtr<GtkToggleButton> tglWeightThin;
    xoj::util::GObjectSPtr<GtkToggleButton> tglWeightBook;
    xoj::util::GObjectSPtr<GtkToggleButton> tglWeightBold;

    xoj::util::GObjectSPtr<GtkToggleButton> tglStyleItalic;
    xoj::util::GObjectSPtr<GtkToggleButton> tglStyleOblique;

    xoj::util::GObjectSPtr<GtkToggleButton> tglUnderlineSingle;
    xoj::util::GObjectSPtr<GtkToggleButton> tglUnderlineSquiggle;
    xoj::util::GObjectSPtr<GtkToggleButton> tglUnderlineDouble;
    xoj::util::GObjectSPtr<GtkToggleButton> tglStrikethrough;
    xoj::util::GObjectSPtr<GtkToggleButton> tglOverlineSingle;

    xoj::util::GObjectSPtr<GtkToggleButton> tglSuperScript;
    xoj::util::GObjectSPtr<GtkToggleButton> tglSubScript;

    xoj::util::GObjectSPtr<GtkButton> removeStyles;


public:
    static constexpr int COLOR_BAR_HEIGHT = 4;
};
