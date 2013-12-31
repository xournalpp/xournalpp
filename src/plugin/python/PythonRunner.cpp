#include "PythonRunner.h"
#include <Python.h>

#include "bindings/PyXournal.h"

class ScriptData
{
public:
	ScriptData(String name, String function, String parameter)
	{
		this->name = name;
		this->function = function;
		this->parameter = parameter;
	}

public:
	String name;
	String function;
	String parameter;
};

PythonRunner* PythonRunner::instance = NULL;

PythonRunner::PythonRunner(Control* control)
{
	XOJ_INIT_TYPE(PythonRunner);

	this->control = control;
	this->pythonInitialized = false;

	this->callbackId = 0;

	this->mutex = g_mutex_new();
	this->scripts = NULL;
}

PythonRunner::~PythonRunner()
{
	XOJ_CHECK_TYPE(PythonRunner);

	if (this->callbackId)
	{
		g_source_remove(this->callbackId);
		this->callbackId = 0;
	}

	if (this->pythonInitialized)
	{
		Py_Finalize();
	}

	g_mutex_free(this->mutex);
	this->mutex = NULL;

	for (GList* l = this->scripts; l != NULL; l = l->next)
	{
		ScriptData* s = (ScriptData*) l->data;
		delete s;
	}
	g_list_free(this->scripts);
	this->scripts = NULL;

	XOJ_RELEASE_TYPE(PythonRunner);
}

void PythonRunner::initPythonRunner(Control* control)
{
	if (instance)
	{
		g_warning("PythonRunner already initialized!");
		return;
	}

	instance = new PythonRunner(control);
}

void PythonRunner::releasePythonRunner()
{
	if (instance)
	{
		delete instance;
		instance = NULL;
	}
}

void PythonRunner::runScript(String name, String function, String parameter)
{
	if(name.isEmpty() || function.isEmpty())
	{
		g_warning("runScript::name (%s) and function (%s) should not be empty!",
		          name.c_str(), function.c_str());
		return;
	}

	if (instance == NULL)
	{
		g_warning("PythonRunner not initialized!");
		return;
	}

	g_mutex_lock(instance->mutex);

	if (instance->callbackId == 0)
	{
		instance->callbackId = g_idle_add((GSourceFunc) scriptRunner, instance);
	}

	instance->scripts = g_list_append(instance->scripts, new ScriptData(name,
	                                                                    function, parameter));

	g_mutex_unlock(instance->mutex);
}

bool PythonRunner::scriptRunner(PythonRunner* runner)
{
	g_mutex_lock(runner->mutex);
	GList* first = g_list_first(runner->scripts);
	ScriptData* data = (ScriptData*) first->data;
	runner->scripts = g_list_delete_link(runner->scripts, first);
	g_mutex_unlock(runner->mutex);

	runner->runScriptInt(data->name, data->function, data->parameter);

	delete data;

	bool callAgain = false;
	g_mutex_lock(runner->mutex);
	if (runner->scripts)
	{
		callAgain = true;
	}
	else
	{
		callAgain = false;
		runner->callbackId = 0;
	}
	g_mutex_unlock(runner->mutex);

	return callAgain;
}

void PythonRunner::initPython()
{
	XOJ_CHECK_TYPE(PythonRunner);

	if (this->pythonInitialized)
	{
		return;
	}
	Py_SetProgramName("Xournal");

	Py_Initialize();

	PyXournal_initPython(this->control);

	char buffer[512] = { 0 };
	const char* path = getcwd(buffer, sizeof(buffer));
	g_return_if_fail(path != NULL);

	PyObject* sysModule = PyImport_ImportModule("sys");
	g_return_if_fail(sysModule != NULL);
	PyObject* pathObject = PyObject_GetAttrString(sysModule, "path");
	g_return_if_fail(pathObject != NULL);

	String p = path;
	p += "/../testing";
	PyObject* ret = PyObject_CallMethod(pathObject, "append", "s", p.c_str());
	Py_DecRef(ret);

	p = path;
	p += "/testing";
	ret = PyObject_CallMethod(pathObject, "append", "s", p.c_str());
	Py_DecRef(ret);

	Py_DecRef(pathObject);
	Py_DecRef(sysModule);
}

void PythonRunner::runScriptInt(String path, String function, String parameter)
{
	XOJ_CHECK_TYPE(PythonRunner);

	if(path.isEmpty() || function.isEmpty())
	{
		g_warning("runScriptInt::path and function should not be empty!");
		return;
	}

	initPython();

	PyObject* pName = PyString_FromString(path.c_str());
	/* Error checking of pName left out */

	PyObject* pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL)
	{
		PyObject* pFunc = PyObject_GetAttrString(pModule, function.c_str());
		/* pFunc is a new reference */

		if (pFunc && PyCallable_Check(pFunc))
		{
			PyObject* pArgs = NULL;

			if (parameter.c_str() == NULL)
			{
				pArgs = PyTuple_New(0);
			}
			else
			{
				pArgs = PyTuple_New(1);
				PyTuple_SetItem(pArgs, 0, PyString_FromString(parameter.c_str()));
			}

			PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
			Py_DECREF(pArgs);
			if (pValue != NULL)
			{
				printf("Result of call: %ld\n", PyInt_AsLong(pValue));
				Py_DECREF(pValue);
			}
			else
			{
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr, "Call failed\n");
			}
		}
		else
		{
			if (PyErr_Occurred())
			{
				PyErr_Print();
			}
			fprintf(stderr, "Cannot find function \"%s\"\n", function.c_str());
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else
	{
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", path.c_str());
	}
}
