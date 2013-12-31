/*
 * Xournal++
 *
 * Runs Python scripts for testing or plugins
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


#ifndef __PYTHONRUNNER_H__
#define __PYTHONRUNNER_H__

#include <String.h>
#include <XournalType.h>

class Control;

class PythonRunner
{
private:
	PythonRunner(Control* control);
	virtual ~PythonRunner();

public:
	static void initPythonRunner(Control* control);
	static void releasePythonRunner();

	static void runScript(String name, String function, String parameter = NULL);

private:
	static bool scriptRunner(PythonRunner* runner);

private:
	void runScriptInt(String path, String function, String parameter);

	void initPython();

private:
	XOJ_TYPE_ATTRIB;

	static PythonRunner* instance;

	GMutex* mutex;

	GList* scripts;

	bool pythonInitialized;
	Control* control;

	int callbackId;
};

#endif /* __PYTHONRUNNER_H__ */
