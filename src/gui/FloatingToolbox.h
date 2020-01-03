/*
 * Xournal++
 *
 * The Floating Toolbox
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include "GladeGui.h"

class MainWindow;

enum FloatingToolBoxState { recalcSize = 0, configuration, noChange };

/************************
 * FloatingToolbox:
 *  Uses a GtkOverlay to show a widget containing toolbars.
 *  The getOverlayPosition callback is used to position the toolbox widget relative to the users input device.
 *  The ui/main.glade file defines the floatingToolbox as a GtkRevealer widget - attempts to use another widget
 *  had issues with leave-notify-event.
 */

class FloatingToolbox {
public:
    FloatingToolbox(MainWindow* theMainWindow, GtkOverlay* overlay);
    virtual ~FloatingToolbox();

public:
    /**
     *  show(x,y): Show Toolbox at centered at x,y relative to main window.
     */
    void show(int x, int y);

    /**
     * showForConfiguration
     * i.e. appear in fixed position in top left, extra space and do not hide automatically.
     */
    void showForConfiguration();

    void hide();

    /**
     * flagRecalculateSizeRequired(): trigger recalc size on next getOverlayPosition. Used when new toolbars loaded.
     */
    void flagRecalculateSizeRequired();

private:
    /**
     * Callback for positioning overlayed floating menu
     */
    static gboolean getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                       FloatingToolbox* self);

    /**
     * Callback to hide floating Toolbar when mouse leaves it
     */
    static void handleLeaveFloatingToolbox(GtkWidget* floatingToolbox, GdkEvent* event, FloatingToolbox* self);

    /**
     * Show the Floating Toolbox
     * ... but hide some labels depending on conditions.
     */
    void show();

    /**
     * check if user has assigned a button to activate, or has put tools in the FloatingToolbox.
     */
    bool floatingToolboxActivated();

    /**
     * Return number of widgets in toolbox.
     * Note this includes non-tools such as spacers and separators.
     */
    int countWidgets();


private:
    MainWindow* mainWindow;
    GtkWidget* floatingToolbox;

    /**
     * Communicating with getOverlayPosition callback
     * */
    int floatingToolboxX = 0;
    int floatingToolboxY = 0;
    FloatingToolBoxState floatingToolboxState = recalcSize;
    int numWidgets = 0;
};
