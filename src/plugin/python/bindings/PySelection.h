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

#ifndef __PYSELECTION_H__
#define __PYSELECTION_H__

class Control;
#include <Python.h>

extern "C"
{

void initselection();

PyObject* newPySelection(PyObject* xournal);

}

#endif /* __PYSELECTION_H__ */
