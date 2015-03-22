#include "PyDocument.h"
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
} PyDocument;

static PyObject*
PyDocument_getPageCount(PyDocument* self)
{
	Document* doc = self->control->getDocument();

	return PyLong_FromLong(doc->getPageCount());
}

static PyMethodDef PyDocument_methods[] = {
	{ "getPageCount", (PyCFunction) PyDocument_getPageCount, METH_VARARGS, "Gets the page count" },
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

static void PyDocument_dealloc(PyDocument* self)
{
	self->ob_type->tp_free((PyObject*) self);
}

static PyObject*
PyDocument_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	PyDocument* self;

	self = (PyDocument*) type->tp_alloc(type, 0);
	if (self != NULL)
	{
		self->control = NULL;
	}

	return (PyObject*) self;
}

static int Selection_init(PyDocument* self, PyObject* args, PyObject* kwds)
{
	return 0;
}

static int PyDocument_init(PyDocument* self, PyObject* args, PyObject* kwds)
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

static PyMemberDef PyDocument_members[] = {
	{ NULL } /* Sentinel */
};

static PyTypeObject SelectionType = {
	PyObject_HEAD_INIT(NULL)0, /*ob_size*/
	"xournal.Document", /*tp_name*/
	sizeof (PyDocument), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyDocument_dealloc, /*tp_dealloc*/
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
	PyDocument_methods, /* tp_methods */
	PyDocument_members, /* tp_members */
	0, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	(initproc) PyDocument_init, /* tp_init */
	0, /* tp_alloc */
	PyDocument_new, /* tp_new */
};

static PyMethodDef module_methods[] = {
	{NULL } /* Sentinel */
};

PyObject* newPyDocument(PyObject* xournal)
{
	PyObject* args = Py_BuildValue("(O)", xournal);
	PyObject* obj = PyObject_CallObject((PyObject*) & SelectionType, args);
	return obj;
}

void initdocument()
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
	PyModule_AddObject(m, "Document", (PyObject*) & SelectionType);
}

#ifdef __cplusplus
}
#endif
