/*
 * Xournal++
 *
 * Python bindings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __PYXOURNAL_H__
#define __PYXOURNAL_H__

class Control;

#include <Python.h>

extern "C"
{

void PyXournal_initPython(Control* control);
Control* PyXournal_getControl(PyObject* obj);

bool PyXournal_Check(PyObject* obj);

}

#endif /* __PYXOURNAL_H__ */
