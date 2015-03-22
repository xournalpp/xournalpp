/*
 * Xournal++
 *
 * Adapter to call progress interface from outside the GTK Main thread
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SYNCHRONIZEDPROGRESSLISTENER_H__
#define __SYNCHRONIZEDPROGRESSLISTENER_H__

#include "ProgressListener.h"
#include <XournalType.h>

class SynchronizedProgressListener : public ProgressListener
{
public:
	SynchronizedProgressListener(ProgressListener* target);
	virtual ~SynchronizedProgressListener();

public:
	virtual void setMaximumState(int max);
	virtual void setCurrentState(int state);

private:
	static bool setMaxCallback(SynchronizedProgressListener* listener);
	static bool setCurrentCallback(SynchronizedProgressListener* listener);

public:
	XOJ_TYPE_ATTRIB;

	ProgressListener* target;

	int maxIdleId;
	int currentIdleId;

	int max;
	int current;
};

#endif /* __SYNCHRONIZEDPROGRESSLISTENER_H__ */
