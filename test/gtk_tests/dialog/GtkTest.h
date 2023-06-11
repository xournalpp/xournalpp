//
// Created by hermannt on 11.06.23.
//

#ifndef XOURNALPP_GTKTEST_H
#define XOURNALPP_GTKTEST_H

#include <gtest/gtest.h>
#include <gtk/gtk.h>

class GtkTest: public ::testing::Test {
protected:
    GtkApplication* app;
    int argn;
    char** argv;

    virtual void runTest(GtkApplication* app) = 0;

    // Setting up the testing environment
    void SetUp() override;

    // This the callback in which the actual test is run
    // It needs to be a callback because it requires the GtkApplication to be running already.
    static void applicationCallback(GtkApplication* app, gpointer userData);
};

#endif  // XOURNALPP_GTKTEST_H
