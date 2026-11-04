#include <cstring>

#include "control/Control.h"
#include "control/XournalMain.h"
#include "control/settings/Settings.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "gui/dialog/SettingsDialog.h"

#include "ScribbleGtkTest.h"
#include "config-test.h"

namespace {
GtkWidget* findWidget(GtkWidget* parent, const char* id) {
    const char* name = gtk_buildable_get_name(GTK_BUILDABLE(parent));
    if (name && std::strcmp(name, id) == 0) {
        return parent;
    }
    if (!GTK_IS_CONTAINER(parent)) {
        return nullptr;
    }
    auto* children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget* found = nullptr;
    for (auto* child = children; child && !found; child = child->next) {
        found = findWidget(GTK_WIDGET(child->data), id);
    }
    g_list_free(children);
    return found;
}

class ScribbleSettingsDialogTest: public ScribbleGtkTest {
    void runTest(GtkApplication*) override {
        XournalMain::initLocalisation();
        auto& control = createControl();
        createWindow();
        auto* settings = control.getSettings();
        settings->latexSettings.globalTemplatePath = fs::path(PROJECT_SOURCE_DIR) / "resources/default_template.tex";
        settings->setScribbleToEraseEnabled(false);
        unsigned saves = 0;
        for (int step = 0; step < 3; ++step) {
            SCOPED_TRACE(step);
            const bool before = step != 0;
            EXPECT_EQ(settings->getScribbleToEraseEnabled(), before);
            SettingsDialog dialog(&resources, settings, &control, {}, [&]() { ++saves; });
            auto* checkbox = findWidget(GTK_WIDGET(dialog.getWindow()), "cbScribbleToEraseEnabled");
            ASSERT_TRUE(GTK_IS_TOGGLE_BUTTON(checkbox));
            EXPECT_EQ(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)), before);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), !before);
            auto* button = findWidget(GTK_WIDGET(dialog.getWindow()), step == 1 ? "btCancel" : "btOk");
            ASSERT_TRUE(GTK_IS_BUTTON(button));
            gtk_button_clicked(GTK_BUTTON(button));
            EXPECT_EQ(settings->getScribbleToEraseEnabled(), step != 2);
        }
        EXPECT_EQ(saves, 2);

        settings->setScribbleToEraseEnabled(true);
        settings->setScribbleSensitivity(75);
        settings->setScribbleMinimumPasses(2);
        for (int step = 0; step < 4; ++step) {
            SettingsDialog dialog(&resources, settings, &control, {}, [&]() { ++saves; });
            auto find = [&](const char* id) { return findWidget(GTK_WIDGET(dialog.getWindow()), id); };
            auto* sensitivity = find("spScribbleSensitivity");
            auto* passes = find("spScribbleMinimumPasses");
            auto* advanced = find("exScribbleAdvanced");
            auto* enabled = find("cbScribbleToEraseEnabled");
            ASSERT_TRUE(GTK_IS_SPIN_BUTTON(sensitivity));
            ASSERT_TRUE(GTK_IS_SPIN_BUTTON(passes));
            ASSERT_TRUE(GTK_IS_EXPANDER(advanced));
            gtk_expander_set_expanded(GTK_EXPANDER(advanced), true);
            EXPECT_EQ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sensitivity)), step < 2 ? 75 : 80);
            EXPECT_EQ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(passes)), step < 2 ? 2 : 3);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enabled), false);
            EXPECT_FALSE(gtk_widget_get_sensitive(advanced));
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enabled), true);
            EXPECT_TRUE(gtk_widget_get_sensitive(advanced));
            if (step >= 2) {
                gtk_button_clicked(GTK_BUTTON(find("btScribbleDefaults")));
                EXPECT_EQ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sensitivity)), 50);
                EXPECT_EQ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(passes)), 4);
                EXPECT_EQ(settings->getScribbleSettings(), (ScribbleSettings{80, 3}));
            } else {
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(sensitivity), 80);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(passes), 3);
            }
            gtk_button_clicked(GTK_BUTTON(find(step % 2 == 0 ? "btCancel" : "btOk")));
            const ScribbleSettings expected = step == 0 ? ScribbleSettings{75, 2} :
                                              step < 3  ? ScribbleSettings{80, 3} :
                                                          ScribbleSettings{};
            EXPECT_EQ(settings->getScribbleSettings(), expected);
        }
        EXPECT_EQ(saves, 4);
        control.enableAutosave(false);
    }
};
}  // namespace

TEST_F(ScribbleSettingsDialogTest, loadsSavesAndCancelsPreferenceInActualDialog) {}
