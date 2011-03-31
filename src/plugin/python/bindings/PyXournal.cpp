#include "PyXournal.h"
#include "../../../control/Control.h"

#include <Python.h>
#include "structmember.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

typedef struct {
	PyObject_HEAD
	Control * control;
	PyObject * undoRedoHandler;
} PyXournal;

static Control * PyXournal_control = NULL;

void initxournal();

void PyXournal_initPython(Control * control) {
	PyXournal_control = control;

	initxournal();
}

static void PyXournal_dealloc(PyXournal* self) {
	self->ob_type->tp_free((PyObject*) self);
}

static PyObject *
PyXournal_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	PyXournal *self;

	self = (PyXournal *) type->tp_alloc(type, 0);
	if (self != NULL) {
		self->control = PyXournal_control;
		self->undoRedoHandler = PyLong_FromLong(5);
	}

	return (PyObject *) self;
}

static int PyXournal_init(PyXournal *self, PyObject *args, PyObject *kwds) {
	return 0;
}

static PyMemberDef PyXournal_members[] = {
	{ "undoRedoHandler", T_OBJECT, offsetof(PyXournal, undoRedoHandler), 0, "Undo- / Redohandler" },

	{ NULL } /* Sentinel */
};

static PyObject *
PyXournal_newFile(PyXournal * self, PyObject * args) {
	PyObject * force = NULL;

	if (!PyArg_ParseTuple(args, "|o", &force)) {
		PyErr_SetString(PyExc_AttributeError, "[Boolean]");
		return NULL;
	}

	if (!PyBool_Check(force)) {
		PyErr_SetString(PyExc_AttributeError, "[Boolean]");
		return NULL;
	}

	if (PyObject_IsTrue(force)) {
		self->control->getUndoRedoHandler()->clearContents();
	}

	if (self->control->newFile()) {
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject *
PyXournal_saveFile(PyXournal * self, PyObject * args) {
	char * path = NULL;

	if (!PyArg_ParseTuple(args, "s", &path)) {
		PyErr_SetString(PyExc_AttributeError, "String");
		return NULL;
	}

	self->control->getDocument()->setFilename(path);

	if (self->control->save(true)) {
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject *
PyXournal_openFile(PyXournal * self, PyObject * args) {
	char * path = NULL;
	int scrollToPage = -1;

	if (!PyArg_ParseTuple(args, "s|i", &path, &scrollToPage)) {
		PyErr_SetString(PyExc_AttributeError, "String [int]");
		return NULL;
	}

	if (self->control->openFile(path, scrollToPage)) {
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject *
PyXournal_setSelectedTool(PyXournal * self, PyObject * args) {
	int tool = -1;

	if (!PyArg_ParseTuple(args, "i", &tool)) {
		PyErr_SetString(PyExc_AttributeError, "int");
		return NULL;
	}

	if (tool < TOOL_PEN || tool > TOOL_HAND) {
		PyErr_SetString(PyExc_AttributeError, "Tool out of range, please use the constants TOOL_*");
	}

	self->control->getToolHandler()->selectTool((ToolType) tool);

	Py_RETURN_NONE;
}

static PyObject *
PyXournal_getSelectedTool(PyXournal* self) {
	ToolType tt = self->control->getToolHandler()->getToolType();

	return PyLong_FromLong(tt);
}

static PyObject *
PyXournal_mousePressed(PyXournal * self, PyObject * args) {
	int x = -1;
	int y = -1;

	if (!PyArg_ParseTuple(args, "ii", &x, &y)) {
		PyErr_SetString(PyExc_AttributeError, "int int");
		return NULL;
	}

	// TODO: implememnt
	g_warning("NOT IMPLEMENTED!\n");

	Py_RETURN_NONE;
}

static PyObject *
PyXournal_mouseMoved(PyXournal * self, PyObject * args) {
	int x = -1;
	int y = -1;

	if (!PyArg_ParseTuple(args, "ii", &x, &y)) {
		PyErr_SetString(PyExc_AttributeError, "int int");
		return NULL;
	}

	// TODO: implememnt
	g_warning("NOT IMPLEMENTED!\n");

	Py_RETURN_NONE;
}

static PyObject *
PyXournal_mouseReleased(PyXournal * self) {
	// TODO: implememnt
	g_warning("NOT IMPLEMENTED!\n");

	Py_RETURN_NONE;
}

static PyObject *
PyXournal_getUndoRedoHandler(PyXournal * self) {
	// TODO: implememnt
	g_warning("NOT IMPLEMENTED!\n");

	Py_RETURN_NONE;
}

static PyObject *
PyXournal_getSelectedPage(PyXournal * self) {
	int pageNo = self->control->getCurrentPageNo();

	return PyLong_FromLong(pageNo);
}

static PyObject *
PyXournal_getDocument(PyXournal * self) {
	// TODO: implememnt
	g_warning("NOT IMPLEMENTED!\n");

	Py_RETURN_NONE;
}

static PyObject *
PyXournal_selectPage(PyXournal * self, PyObject * args) {
	int page = -1;

	if (!PyArg_ParseTuple(args, "i", &page)) {
		PyErr_SetString(PyExc_AttributeError, "int");
		return NULL;
	}

	self->control->getScrollHandler()->scrollToPage(page, 0);

	Py_RETURN_NONE;
}

static PyMethodDef PyXournal_methods[] = {
	{ "setSelectedTool", (PyCFunction) PyXournal_setSelectedTool, METH_VARARGS, "Selects a tool (see constatns TOOL_*" },
	{ "getSelectedTool", (PyCFunction) PyXournal_getSelectedTool, METH_NOARGS, "Return the selected tool" },
	{ "newFile", (PyCFunction) PyXournal_newFile, METH_VARARGS, "Create a new document" },
	{ "saveFile", (PyCFunction) PyXournal_saveFile, METH_VARARGS, "Save a document as" },
	{ "openFile", (PyCFunction) PyXournal_openFile, METH_VARARGS, "Opens a file" },
	{ "mousePressed", (PyCFunction) PyXournal_mousePressed, METH_VARARGS, "Simulate a mouse press on the current view" },
	{ "mouseMoved", (PyCFunction) PyXournal_mouseMoved, METH_VARARGS, "Simulate a mouse move on the current view" },
	{ "mouseReleased", (PyCFunction) PyXournal_mouseReleased, METH_NOARGS, "Simulate a mouse release" },
	{ "getUndoRedoHandler", (PyCFunction) PyXournal_getUndoRedoHandler, METH_NOARGS, "Gets the Undo- / Redohandler" },
	{ "getDocument", (PyCFunction) PyXournal_getDocument, METH_NOARGS, "Return the Xournal Document" },
	{ "getSelectedPage", (PyCFunction) PyXournal_getSelectedPage, METH_NOARGS, "Gets the selected page ID (first Page: 0)" },
	{ "selectPage", (PyCFunction) PyXournal_selectPage, METH_VARARGS, "Sets the selected page (first Page: 0)" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
//	{ "xxxxxxxxxxxxx", (PyCFunction) xxxxxxxxxxxxxxxxx, METH_VARARGS, "Xxxxxxxxxxxxxxxx" },
	{ NULL } /* Sentinel */
};

static PyTypeObject XournalType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"xournal.Xournal", /*tp_name*/
	sizeof(PyXournal), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor)PyXournal_dealloc, /*tp_dealloc*/
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
	PyXournal_methods, /* tp_methods */
	PyXournal_members, /* tp_members */
	0, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	(initproc)PyXournal_init, /* tp_init */
	0, /* tp_alloc */
	PyXournal_new, /* tp_new */
};

static PyMethodDef module_methods[] = { { NULL } /* Sentinel */
};


#define ADD_TOOL(tool) PyDict_SetItemString(XournalType.tp_dict, #tool, PyLong_FromLong(tool));

void initxournal() {
	PyObject* m;

	if (PyType_Ready(&XournalType) < 0)
		return;

	ADD_TOOL(TOOL_PEN);
	ADD_TOOL(TOOL_ERASER);
	ADD_TOOL(TOOL_HILIGHTER);
	ADD_TOOL(TOOL_TEXT);
	ADD_TOOL(TOOL_IMAGE);
	ADD_TOOL(TOOL_SELECT_RECT);
	ADD_TOOL(TOOL_SELECT_REGION);
	ADD_TOOL(TOOL_SELECT_OBJECT);
	ADD_TOOL(TOOL_VERTICAL_SPACE);
	ADD_TOOL(TOOL_HAND);

	m = Py_InitModule3("xournal", module_methods, "Xournal api modul");

	if (m == NULL)
		return;

	Py_INCREF(&XournalType);
	PyModule_AddObject(m, "Xournal", (PyObject *) &XournalType);
}

#ifdef __cplusplus
}
#endif
