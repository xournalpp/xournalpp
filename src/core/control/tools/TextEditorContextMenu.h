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

private:
    void create();

private:
    Control* control;
    TextEditor* editor;
    XojPageView* pageView;
    GtkWidget* xournalWidget;

    /**
     * UI Elements
     */
    GtkPopover* contextMenu;

    GtkFontButton* fontBtn;

    GtkColorButton* ftColorBtn;
    GtkColorButton* bgColorBtn;

    GtkToggleButton* alignLeftTgl;
    GtkToggleButton* alignCenterTgl;
    GtkToggleButton* alignRightTgl;
};
