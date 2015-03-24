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

#ifndef __PYDOCUMENT_H__
#define __PYDOCUMENT_H__

class Control;
#include <Python.h>

extern "C"
{

void initdocument();

PyObject* newPyDocument(PyObject* xournal);

}

#endif /* __PYDOCUMENT_H__ */
