#include "PyUndoRedoHandler.h"
#include "PyXournal.h"

#include "../../../control/Control.h"
#include "../../../gui/XournalView.h"

#include <Python.h>
#include "structmember.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	PyObject_HEAD
	Control* control;
} PyUndoRedoHandler;

static PyObject*
PyUndoRedoHandler_canUndo(PyUndoRedoHandler* self)
{
	UndoRedoHandler* undo = self->control->getUndoRedoHandler();

	if (undo->canUndo())
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject*
PyUndoRedoHandler_canRedo(PyUndoRedoHandler* self)
{
	UndoRedoHandler* undo = self->control->getUndoRedoHandler();

	if (undo->canRedo())
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject*
PyUndoRedoHandler_undo(PyUndoRedoHandler* self)
{
	UndoRedoHandler* undo = self->control->getUndoRedoHandler();
	undo->undo();
	Py_RETURN_NONE;
}

static PyObject*
PyUndoRedoHandler_redo(PyUndoRedoHandler* self)
{
	UndoRedoHandler* undo = self->control->getUndoRedoHandler();
	undo->redo();
	Py_RETURN_NONE;
}

static PyObject*
PyUndoRedoHandler_getUndoItemTypeOnStack(PyUndoRedoHandler* self)
{
	UndoRedoHandler* undo = self->control->getUndoRedoHandler();

	const char* name = undo->getUndoStackTopTypeName();

	if (name)
	{
		return PyString_FromString(name);
	}

	Py_RETURN_NONE;
}

static PyObject*
PyUndoRedoHandler_getRedoItemTypeOnStack(PyUndoRedoHandler* self)
{
	UndoRedoHandler* undo = self->control->getUndoRedoHandler();

	const char* name = undo->getRedoStackTopTypeName();

	if (name)
	{
		return PyString_FromString(name);
	}

	Py_RETURN_NONE;
}

static PyMethodDef PyUndoRedoHandler_methods[] = {
	{ "canUndo", (PyCFunction) PyUndoRedoHandler_canUndo, METH_NOARGS, "If there is something to undo" },
	{ "canRedo", (PyCFunction) PyUndoRedoHandler_canRedo, METH_NOARGS, "If there is something to redo" },
	{ "undo", (PyCFunction) PyUndoRedoHandler_undo, METH_NOARGS, "Undo the last operation" },
	{ "redo", (PyCFunction) PyUndoRedoHandler_redo, METH_NOARGS, "Redo the last operation" },
	{ "getUndoItemTypeOnStack", (PyCFunction) PyUndoRedoHandler_getUndoItemTypeOnStack, METH_NOARGS, "Return the name of the item on top of the undo stack" },
	{ "getRedoItemTypeOnStack", (PyCFunction) PyUndoRedoHandler_getRedoItemTypeOnStack, METH_NOARGS, "Return the name of the item on top of the redo stack" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
	{ NULL } /* Sentinel */
};

static void PyUndoRedoHandler_dealloc(PyUndoRedoHandler* self)
{
	self->ob_type->tp_free((PyObject*) self);
}

static PyObject*
PyUndoRedoHandler_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	PyUndoRedoHandler* self;

	self = (PyUndoRedoHandler*) type->tp_alloc(type, 0);
	if (self != NULL)
	{
		self->control = NULL;
	}

	return (PyObject*) self;
}

static int Selection_init(PyUndoRedoHandler* self, PyObject* args,
						  PyObject* kwds)
{
	return 0;
}

static int PyUndoRedoHandler_init(PyUndoRedoHandler* self, PyObject* args,
								  PyObject* kwds)
{
	PyObject* xournal = NULL;

	static char* kwlist[] = { "xournal", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &xournal))
	{
		return -1;
	}

	self->control = PyXournal_getControl(xournal);

	if (self->control == NULL)
	{
		return -1;
	}

	return 0;
}

static PyMemberDef PyUndoRedoHandler_members[] = {
	{ NULL } /* Sentinel */
};

static PyTypeObject UndoRedoHandlerType = {
	PyObject_HEAD_INIT(NULL)0, /*ob_size*/
	"xournal.UndoRedo", /*tp_name*/
	sizeof (PyUndoRedoHandler), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyUndoRedoHandler_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	0, /*tp_getattro*/
	0, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"Xournal object", /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	PyUndoRedoHandler_methods, /* tp_methods */
	PyUndoRedoHandler_members, /* tp_members */
	0, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	(initproc) PyUndoRedoHandler_init, /* tp_init */
	0, /* tp_alloc */
	PyUndoRedoHandler_new, /* tp_new */
};

static PyMethodDef module_methods[] = {
	{NULL } /* Sentinel */
};

PyObject* newPyUndoRedoHandler(PyObject* xournal)
{
	PyObject* args = Py_BuildValue("(O)", xournal);
	PyObject* obj = PyObject_CallObject((PyObject*) & UndoRedoHandlerType, args);
	return obj;
}

void initundoredohandler()
{
	if (PyType_Ready(&UndoRedoHandlerType) < 0)
	{
		return;
	}

	PyObject* m = Py_InitModule3("xournal", module_methods, "Xournal API modul");

	if (m == NULL)
	{
		return;
	}

	Py_INCREF(&UndoRedoHandlerType);
	PyModule_AddObject(m, "UndoRedo", (PyObject*) & UndoRedoHandlerType);
}

#ifdef __cplusplus
}
#endif
