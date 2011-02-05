/*
 * Xournal++
 *
 * Controls background tasks
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#ifndef __BACKGROUNDTHREADHANDLER_H__
#define __BACKGROUNDTHREADHANDLER_H__

#include <glib.h>

class Runnable;
class Control;

class BackgroundThreadHandler {
public:
	BackgroundThreadHandler(Control * control);
	virtual ~BackgroundThreadHandler();

	/**
	 * Runs a runnable in Background, the runnable will be deleted after it has finished
	 */
	void run(Runnable * runnable);

	bool isRunning();
	void stop();

private:
	static int threadCallback(BackgroundThreadHandler * th);
	static bool finished(BackgroundThreadHandler * th);

private:
	Control * control;
	Runnable * runnable;

	GThread * thread;

	bool cancel;
};

class Runnable {
public:
	virtual bool run(bool * cancel) = 0;

};

#endif /* __BACKGROUNDTHREADHANDLER_H__ */
