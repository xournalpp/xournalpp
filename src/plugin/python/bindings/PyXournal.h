/*
 * Xournal++
 *
 * Python bindings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
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
