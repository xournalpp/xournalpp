/*
 * Xournal++
 *
 * Output streams for writing
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __OUTPUTSTREAM_H__
#define __OUTPUTSTREAM_H__

#include "../util/String.h"
#include <zlib.h>

class OutputStream {
public:
	OutputStream();
	virtual ~OutputStream();

public:
	virtual void write(const char * data);
	virtual void write(const char * data, int len) = 0;
	virtual void write(const String & str);

	virtual void close() = 0;
};

class GzOutputStream : public OutputStream {
public:
	GzOutputStream(String filename);
	~GzOutputStream();

	virtual void write(const char * data, int len);

	virtual void close();


	String getLastError();
private:
	gzFile fp;

	String error;

	String target;
	String filename;
};


#endif /* __OUTPUTSTREAM_H__ */
