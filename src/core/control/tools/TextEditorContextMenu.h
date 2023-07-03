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

    void reposition();

    void changeFont();
    void changeFtColor();
    void changeBgColor();
    void changeAlignment(TextAlignment align);
    void toggleBoldStyle();
    void toggleItalicStyle();
    void toggleUnderlineStyle();

    gboolean drawFtColorIcon(GtkWidget* src, cairo_t* cr);
    gboolean drawBgColorIcon(GtkWidget* src, cairo_t* cr);

private:
    void create();

private:
    Control* control;
    TextEditor* editor;
    XojPageView* pageView;
    GtkWidget* xournalWidget;

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

    GtkButton* tglBoldBtn;
    GtkButton* tglItalicBtn;
    GtkButton* tglUnderlineBtn;
    GtkButton* expandTextDecoration;

    GtkButton* ftColorBtn;
    GtkWidget* ftColorIcon;
    GtkButton* bgColorBtn;
    GtkWidget* bgColorIcon;

    GtkToggleButton* alignLeftTgl;
    GtkToggleButton* alignCenterTgl;
    GtkToggleButton* alignRightTgl;

public:
    static constexpr int COLOR_BAR_HEIGHT = 4;
};
