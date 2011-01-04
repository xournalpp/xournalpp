/*
 * Xournal++
 *
 * Reference String which is automatically deleted
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __STRINGBUFFER_H__
#define __STRINGBUFFER_H__

#include "String.h"

class StringBuffer {
public:
	StringBuffer(int initialLenght = 100);
	virtual ~StringBuffer();

public:
	void append(String s);
	void append(const char * str);

	void appendFormat(const char * format, ...);

	String toString();

private:
	void reserveBuffer(int len);
	void append(const char * str, int len);

private:
	char * data;
	int pos;
	int lenght;
};

#endif /* __STRINGBUFFER_H__ */
