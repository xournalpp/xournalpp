/*
 * Xournal++
 *
 * A job which saves a Document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SAVEJOB_H__
#define __SAVEJOB_H__

#include "BlockingJob.h"
#include "../../util/String.h"

class SaveJob : public BlockingJob {
public:
	SaveJob(Control * control);
	virtual ~SaveJob();

public:
	virtual void run();

	bool save();

private:
	static void copyProgressCallback(goffset current_num_bytes, goffset total_num_bytes, Control * control);
	bool copyFile(String source, String target);

	void updatePreview();
	virtual void afterRun();

private:
	String copyError;

	String lastError;

};

#endif /* __SAVEJOB_H__ */
