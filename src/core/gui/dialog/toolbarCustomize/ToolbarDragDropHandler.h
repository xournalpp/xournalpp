/*
 * Xournal++
 *
 * Toolbar drag & drop controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>

class Control;
class ToolbarAdapter;
class ToolbarCustomizeDialog;

class ToolbarDragDropHandler {
public:
    ToolbarDragDropHandler(Control* control);
    virtual ~ToolbarDragDropHandler();

public:
    void configure();

    void toolbarConfigDialogClosed();

public:
    void prepareToolbarsForDragAndDrop();
    void clearToolbarsFromDragAndDrop();

private:
    Control* control;

    std::vector<std::unique_ptr<ToolbarAdapter>> toolbars;
    std::unique_ptr<ToolbarCustomizeDialog> customizeDialog;
};
