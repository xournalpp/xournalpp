#include "ScribbleGtkTest.h"

#include "control/Control.h"
#include "gui/MainWindow.h"
#include "util/gtk4_helper.h"

#include "config-test.h"

ScribbleGtkTest::~ScribbleGtkTest() = default;

void ScribbleGtkTest::SetUp() {
    resources.addSearchDirectory(GET_UI_FOLDER);
    resources.addSearchDirectory(GET_RESOURCE_TEMPLATE_FOLDER);
    app = gtk_application_new("com.github.xournalpp.xournalpp.scribble-test", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(app, "activate", G_CALLBACK(applicationCallback), this);
    char name[] = "xournalpp_scribble_test";
    char* arguments[] = {name, nullptr};
    g_application_run(G_APPLICATION(app), 1, arguments);
}

auto ScribbleGtkTest::createControl() -> Control& {
    if (!controller) {
        controller = std::make_unique<Control>(G_APPLICATION(app), &resources);
    }
    return *controller;
}

auto ScribbleGtkTest::createWindow() -> MainWindow& {
    auto& control = createControl();
    mainWindow = std::make_unique<MainWindow>(&resources, &control, app);
    control.initWindow(mainWindow.get(), true);
    mainWindow->populate(&resources);
    control.enableAutosave(false);
    return *mainWindow;
}

void ScribbleGtkTest::TearDown() {
    if (mainWindow) {
        // Finish asynchronous initialization while the test's objects are alive.
        auto* clipboard = gtk_widget_get_clipboard(mainWindow->getWindow());
        GdkAtom* targets = nullptr;
        gint count = 0;
        gtk_clipboard_wait_for_targets(clipboard, &targets, &count);
        g_free(targets);
        while (g_main_context_iteration(nullptr, false)) {}
        // Remove this fixture's notifications from the shared GtkSettings object.
        g_signal_handlers_disconnect_by_data(gtk_widget_get_settings(mainWindow->getWindow()), mainWindow.get());
        mainWindow.reset();
    }
    controller.reset();
    g_object_unref(app);
    app = nullptr;
}
