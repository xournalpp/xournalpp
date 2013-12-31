/*
 * Xournal++
 *
 * Logging class for debugging
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

class Log
{
private:
	Log();
	virtual ~Log();
	Log(const Log&);
	Log& operator =(const Log&);

public:
	static void initlog();
	static void closelog();

	static void trace(const char* callType, const char* clazz, const char* function,
	                  long obj);
};

#endif /* __LOGGER_H__ */
