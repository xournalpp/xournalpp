/*
 * Xournal++
 *
 * Python bindings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class Control;

#include <Python.h>

extern "C"
{

void PyXournal_initPython(Control* control);
Control* PyXournal_getControl(PyObject* obj);

bool PyXournal_Check(PyObject* obj);

}
