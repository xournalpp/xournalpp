/*
 * OutputStream.h
 *
 *  Created on: 14.11.2010
 *      Author: andreas
 */

#ifndef OUTPUTSTREAM_H_
#define OUTPUTSTREAM_H_

#include "../model/String.h"
#include <zlib.h>

class OutputStream {
public:
	OutputStream();
	virtual ~OutputStream();

public:
	virtual void write(const char * data) = 0;
	virtual void write(const String & str);

	virtual void close() = 0;
};

class StdOutputStream : public OutputStream {
public:
	virtual void write(const char * data);

	virtual void close();
};

class GzOutputStream : public OutputStream {
public:
	GzOutputStream(String filename);

	virtual void write(const char * data);

	virtual void close();


	String getLastError();
private:
	gzFile fp;

	String error;

	String target;
	String filename;
};


#endif /* OUTPUTSTREAM_H_ */
