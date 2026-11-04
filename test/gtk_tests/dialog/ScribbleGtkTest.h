#pragma once

#include <memory>

#include "gui/GladeSearchpath.h"

#include "GtkTest.h"

class Control;
class MainWindow;

class ScribbleGtkTest: public GtkTest {
protected:
    ~ScribbleGtkTest() override;
    void SetUp() override;
    void TearDown() override;
    Control& createControl();
    MainWindow& createWindow();

    GladeSearchpath resources;

private:
    std::unique_ptr<Control> controller;
    std::unique_ptr<MainWindow> mainWindow;
};
