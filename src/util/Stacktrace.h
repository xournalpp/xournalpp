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

#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__

#include <iostream>
#include <StringUtils.h>

class Stacktrace
{
private:
	Stacktrace();
	virtual ~Stacktrace();

public:
	static void setExename(string name);

	static void printStracktrace();
	static void printStracktrace(std::ostream& stream);
};

#endif /* __STACKTRACE_H__ */
