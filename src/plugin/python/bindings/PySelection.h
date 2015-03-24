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
