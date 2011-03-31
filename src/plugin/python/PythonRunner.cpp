#include "PythonRunner.h"
#include <Python.h>

#include "bindings/PyXournal.h"

PythonRunner::PythonRunner(Control * control) {
	XOJ_INIT_TYPE(PythonRunner);

	this->control = control;
	this->pythonInitialized = false;
}

PythonRunner::~PythonRunner() {
	XOJ_CHECK_TYPE(PythonRunner);

	if(this->pythonInitialized) {
		Py_Finalize();
	}

	XOJ_RELEASE_TYPE(PythonRunner);
}

void PythonRunner::addPath(String path, String & cmd) {
	XOJ_CHECK_TYPE(PythonRunner);

	path = path.replace("\"", "\\\"");
	cmd += "sys.path.append(\"";
	cmd += path;
	cmd += "\")\n";
}

void PythonRunner::initPython() {
	XOJ_CHECK_TYPE(PythonRunner);

	if(this->pythonInitialized) {
		return;
	}
    Py_SetProgramName("Xournal");

    Py_Initialize();

	PyXournal_initPython(this->control);

	char buffer[512] = { 0 };
	const char * path = getcwd(buffer, sizeof(buffer));

	String cmd = "import sys\n";

	if (path != NULL) {
		String p = path;
		p += "/../testing";
		addPath(p, cmd);

		p = path;
		p += "/testing";
		addPath(p, cmd);
	}

	PyRun_SimpleString(cmd.c_str());
}

void PythonRunner::runScript(String path, String function) {
	XOJ_CHECK_TYPE(PythonRunner);

	initPython();

	PyObject * pName = PyString_FromString(path.c_str());
	/* Error checking of pName left out */

	PyObject * pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		PyObject * pFunc = PyObject_GetAttrString(pModule, function.c_str());
		/* pFunc is a new reference */

		if (pFunc && PyCallable_Check(pFunc)) {
			PyObject * pArgs = PyTuple_New(0);

			PyObject * pValue = PyObject_CallObject(pFunc, pArgs);
			Py_DECREF(pArgs);
			if (pValue != NULL) {
				printf("Result of call: %ld\n", PyInt_AsLong(pValue));
				Py_DECREF(pValue);
			} else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr, "Call failed\n");
			}
		} else {
			if (PyErr_Occurred()) {
				PyErr_Print();
			}
			fprintf(stderr, "Cannot find function \"%s\"\n", function.c_str());
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	} else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", path.c_str());
	}
}
