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

#pragma once

class ProgressListener
{
public:
	virtual void setMaximumState(int max) = 0;
	virtual void setCurrentState(int state) = 0;
};
