/*
 * Xournal++
 *
 * Base class for Background selection dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkRequisition, GtkWindow

#include "gui/GladeGui.h"  // for GladeGui
#include "util/Util.h"     // for npos

class Document;
class Settings;
class BaseElementView;
class GladeSearchpath;

class BackgroundSelectDialogBase: public GladeGui {
public:
    BackgroundSelectDialogBase(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings,
                               const std::string& glade, const std::string& mainWnd);
    ~BackgroundSelectDialogBase() override;

public:
    Settings* getSettings();
    void show(GtkWindow* parent) override;
    virtual void setSelected(size_t selected);

protected:
    void layout();

private:
    static void sizeAllocate(GtkWidget* widget, GtkRequisition* requisition, BackgroundSelectDialogBase* dlg);

private:
protected:
    Settings* settings = nullptr;
    GtkWidget* scrollPreview = nullptr;
    GtkWidget* layoutContainer = nullptr;

    Document* doc = nullptr;

    /**
     * Selection confirmed
     */
    bool confirmed = false;

    /**
     * Selected image, none if npos
     */
    size_t selected = npos;

    /**
     * To check if the size has real changed
     */
    int lastWidth = 0;

    /**
     * Elements to display
     */
    std::vector<BaseElementView*> elements;
};
