//
// Created by hermannt on 11.06.23.
//

#include "GtkTest.h"

// Setting up the testing environment
void GtkTest::SetUp() {
    argn = 1;
    argv = new char*[2];
    argv[0] = strdup("xournalpp_test");
    argv[1] = nullptr;
    app = gtk_application_new("com.github.xournalpp.xournalpp.test", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(applicationCallback), this);
    g_application_run(G_APPLICATION(app), argn, argv);
}

// This the callback in which the actual test is run
// It needs to be a callback because it requires the GtkApplication to be running already.
void GtkTest::applicationCallback(GtkApplication* app, gpointer userData) {
    auto* test = static_cast<GtkTest*>(userData);

    // run the actual test
    test->runTest(app);

    // Quit the application to avoid waiting indefinitely fo the test to finish
    g_application_quit(G_APPLICATION(app));
}
