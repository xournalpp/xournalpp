/*
 * Xournal++
 *
 * Python bindings for Undo- / Redohandler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

class Control;
#include <Python.h>

extern "C"
{

void initundoredohandler();

PyObject* newPyUndoRedoHandler(PyObject* xournal);

}
