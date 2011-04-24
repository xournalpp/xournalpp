/*
 * Xournal++
 *
 * Serialized input stream
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __OBJECTINPUTSTREAM_H__
#define __OBJECTINPUTSTREAM_H__

#include "InputStreamException.h"

class Serializeable;

class ObjectInputStream {
public:
	ObjectInputStream();
	virtual ~ObjectInputStream();

public:
	bool read(const char * data, int len) throw (InputStreamException);

	void readObject(const char * name) throw (InputStreamException);
	String readObject() throw (InputStreamException);
	String getNextObjectName() throw (InputStreamException);
	void endObject() throw (InputStreamException);

	int readInt() throw (InputStreamException);
	double readDouble() throw (InputStreamException);
	String readString() throw (InputStreamException);

	void readData(void ** data, int * len) throw (InputStreamException);
	cairo_surface_t * readImage() throw (InputStreamException);

	ObjectInputStream & operator >>(Serializeable * s) throw (InputStreamException);

private:
	void checkType(char type) throw (InputStreamException);

	String getType(char type);

private:
	XOJ_TYPE_ATTRIB;

	GString * str;
	int pos;
};

#endif /* __OBJECTINPUTSTREAM_H__ */
