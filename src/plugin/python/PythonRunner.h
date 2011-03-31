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

#include "../../util/String.h"
#include "../../util/XournalType.h"

class Control;

class PythonRunner {
public:
	PythonRunner(Control * control);
	virtual ~PythonRunner();

public:
	void runScript(String path, String function);

private:
	void addPath(String path, String & cmd);

	void initPython();

private:
	XOJ_TYPE_ATTRIB;

	bool pythonInitialized;
	Control * control;
};

#endif /* __PYTHONRUNNER_H__ */
