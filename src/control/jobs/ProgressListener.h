/*
 * Xournal++
 *
 * Interface for progress state
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __PROGRESSLISTENER_H__
#define __PROGRESSLISTENER_H__

class ProgressListener {
public:
	virtual void setMaximumState(int max) = 0;
	virtual void setCurrentState(int state) = 0;
};

#endif /* __PROGRESSLISTENER_H__ */
