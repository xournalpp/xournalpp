/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "control/actions/ActionRef.h"
#include "enums/Action.enum.h"  // for Action
#include "util/EnumIndexedArray.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

#include "AbstractToolItem.h"  // for AbstractToolItem

class ActionDatabase;
class IconNameHelper;

class DrawingTypeComboToolButton: public AbstractToolItem {
public:
    struct Entry {
        Entry() = default;
        Entry(std::string name, std::string icon, const ActionDatabase& db, Action a);
        std::string name;
        std::string icon;
        ActionRef gAction;
        std::string fullActionName;
    };
    DrawingTypeComboToolButton(std::string id, IconNameHelper& icons, const ActionDatabase& db);

    ~DrawingTypeComboToolButton() override;

public:
    std::string getToolDisplayName() const override;

    enum class Type : size_t {
        RECTANGLE,
        ELLIPSE,
        ARROW,
        DOUBLE_ARROW,
        LINE,
        COORDINATE_SYSTEM,
        SPLINE,
        SHAPE_RECOGNIZER,
        ENUMERATOR_COUNT
    };

protected:
    Widgetry createItem(ToolbarSide side) override;

    GtkWidget* getNewToolIcon() const override;

private:
    template <Type s>
    static void setProminentIconCallback(GObject* action, GParamSpec*, DrawingTypeComboToolButton* self);

protected:
    std::shared_ptr<const EnumIndexedArray<Entry, Type>> entries;
    std::string iconName;
    std::string description;
};
