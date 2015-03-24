/*
 * Xournal++
 *
 * Interface for progress state
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __PROGRESSLISTENER_H__
#define __PROGRESSLISTENER_H__

class ProgressListener
{
public:
	virtual void setMaximumState(int max) = 0;
	virtual void setCurrentState(int state) = 0;
};

#endif /* __PROGRESSLISTENER_H__ */
