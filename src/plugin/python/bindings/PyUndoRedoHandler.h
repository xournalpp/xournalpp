/*
 * Xournal++
 *
 * Python bindings for Undo- / Redohandler
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PYUNDOREDOHANDLER_H_
#define __PYUNDOREDOHANDLER_H_

class Control;
#include <Python.h>

extern "C"
{

void initundoredohandler();

PyObject* newPyUndoRedoHandler(PyObject* xournal);

}

#endif /* PYUNDOREDOHANDLER_H_ */
