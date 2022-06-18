/*
 * Xournal++
 *
 * Wrapper for undo / redo stuff, to move out the logic of the main Control class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

class Control;
class Layer;
class Element;

class UndoRedoController {
private:
    UndoRedoController(Control* control);
    virtual ~UndoRedoController();

private:
    void before();
    void after();

public:
    static void undo(Control* control);
    static void redo(Control* control);

private:
    /**
     * Controller
     */
    Control* control = nullptr;

    /**
     * Layer of the selection before change
     */
    Layer* layer = nullptr;

    /**
     * Selected elements
     */
    std::vector<Element*> elements;
};
