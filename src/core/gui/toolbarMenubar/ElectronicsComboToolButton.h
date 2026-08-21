#pragma once

#include <memory>
#include <string>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "control/ToolEnums.h"
#include "util/raii/GObjectSPtr.h"

#include "AbstractToolItem.h"

class ActionDatabase;

class ElectronicsComboToolButton : public AbstractToolItem {
public:
    struct Entry {
        Entry() = default;
        Entry(std::string name, ElectronicsComponentType type);
        std::string name;
        ElectronicsComponentType type;
    };

    ElectronicsComboToolButton(std::string id, ActionDatabase* db);
    ~ElectronicsComboToolButton() override;

    std::string getToolDisplayName() const override;

protected:
    xoj::util::WidgetSPtr createItem(bool horizontal) override;
    GtkWidget* getNewToolIcon() const override;

private:
    static void onComponentSelected(GtkWidget* widget, gpointer data);

    ActionDatabase* db;
};
