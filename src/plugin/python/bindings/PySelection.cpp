#include "PySelection.h"
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
} PySelection;

#define GET_SELECTION() \
	MainWindow * win = self->control->getWindow(); \
	if(!win) { \
		PyErr_SetString(PyExc_RuntimeError, "No xournal window"); \
		return NULL; \
	} \
	XournalView * xoj = win->getXournal(); \
	EditSelection * selection = xoj->getSelection(); \
	if(!selection) { \
		PyErr_SetString(PyExc_RuntimeError, "No selection"); \
		return NULL; \
	}

static PyObject*
PySelection_existsSelection(PySelection* self)
{
	MainWindow* win = self->control->getWindow();
	if (!win)
	{
		PyErr_SetString(PyExc_RuntimeError, "No xournal window");
		return NULL;
	}
	XournalView* xoj = win->getXournal();
	EditSelection* selection = xoj->getSelection();

	if (selection == NULL)
	{
		Py_RETURN_FALSE;
	}

	Py_RETURN_TRUE;
}

static PyObject*
PySelection_getX(PySelection* self)
{
	GET_SELECTION();

	return PyFloat_FromDouble(selection->getXOnView());
}

static PyObject*
PySelection_getY(PySelection* self)
{
	GET_SELECTION();

	return PyFloat_FromDouble(selection->getYOnView());
}

static PyObject*
PySelection_setX(PySelection* self, PyObject* args)
{
	GET_SELECTION();

	// TODO: LOW PRIO, python: not implemented
	//	selection->mo
	Py_RETURN_NONE;
}

static PyObject*
PySelection_setY(PySelection* self, PyObject* args)
{
	GET_SELECTION();

	// TODO: LOW PRIO, python: not implemented
	//	selection->mo
	Py_RETURN_NONE;
}

static PyObject*
PySelection_getPageId(PySelection* self)
{
	GET_SELECTION();

	return PyFloat_FromDouble(selection->getYOnView());
}

static PyMethodDef PySelection_methods[] = {
	{ "existsSelection", (PyCFunction) PySelection_existsSelection, METH_NOARGS, "Return true if a selection exists, if return false then all other methods will fail" },
	{ "getX", (PyCFunction) PySelection_getY, METH_NOARGS, "Return the X coordinate relative to the page" },
	{ "getY", (PyCFunction) PySelection_getY, METH_NOARGS, "Return the Y coordinate relative to the page" },
	{ "setX", (PyCFunction) PySelection_setX, METH_VARARGS, "Sets the X coordinate for the selection (move the selection) relative to the page" },
	{ "setY", (PyCFunction) PySelection_setY, METH_VARARGS, "Sets the Y coordinate for the selection (move the selection) relative to the page" },
	{ "getPageId", (PyCFunction) PySelection_getPageId, METH_VARARGS, "Returns the page which the selection refer to" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
	{ NULL } /* Sentinel */
};

static void PySelection_dealloc(PySelection* self)
{
	self->ob_type->tp_free((PyObject*) self);
}

static PyObject*
PySelection_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	PySelection* self;

	self = (PySelection*) type->tp_alloc(type, 0);
	if (self != NULL)
	{
		self->control = NULL;
	}

	return (PyObject*) self;
}

static int Selection_init(PySelection* self, PyObject* args, PyObject* kwds)
{
	return 0;
}

static int PySelection_init(PySelection* self, PyObject* args, PyObject* kwds)
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

static PyMemberDef PySelection_members[] = {
	{ NULL } /* Sentinel */
};

static PyTypeObject SelectionType = {
	PyObject_HEAD_INIT(NULL)0, /*ob_size*/
	"xournal.Selection", /*tp_name*/
	sizeof (PySelection), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PySelection_dealloc, /*tp_dealloc*/
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
	PySelection_methods, /* tp_methods */
	PySelection_members, /* tp_members */
	0, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	(initproc) PySelection_init, /* tp_init */
	0, /* tp_alloc */
	PySelection_new, /* tp_new */
};

static PyMethodDef module_methods[] = {
	{NULL } /* Sentinel */
};

PyObject* newPySelection(PyObject* xournal)
{
	PyObject* args = Py_BuildValue("(O)", xournal);
	PyObject* obj = PyObject_CallObject((PyObject*) & SelectionType, args);
	return obj;
}

void initselection()
{
	if (PyType_Ready(&SelectionType) < 0)
	{
		return;
	}

	PyObject* m = Py_InitModule3("xournal", module_methods, "Xournal API modul");

	if (m == NULL)
	{
		return;
	}

	Py_INCREF(&SelectionType);
	PyModule_AddObject(m, "Selection", (PyObject*) & SelectionType);
}

#ifdef __cplusplus
}
#endif
