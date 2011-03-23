/*
 * Xournal++
 *
 * Prints a Stacktrace
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__

#include <stdio.h>

class Stacktrace {
private:
	Stacktrace();
	virtual ~Stacktrace();

public:
	static void setExename(const char * name);

	static void printStracktrace();
	static void printStracktrace(FILE * fp);
};

#endif /* __STACKTRACE_H__ */
