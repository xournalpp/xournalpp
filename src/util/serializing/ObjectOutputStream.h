/*
 * Xournal++
 *
 * Serialized output stream
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __OBJECTOUTPUTSTREAM_H__
#define __OBJECTOUTPUTSTREAM_H__

#include <String.h>
#include <gtk/gtk.h>

class Serializeable;
class ObjectEncoding;

class ObjectOutputStream
{
public:
	ObjectOutputStream(ObjectEncoding* encoder);
	virtual ~ObjectOutputStream();

public:
	void writeObject(const char* name);
	void endObject();

	void writeInt(int i);
	void writeDouble(double d);
	void writeString(const char* str);
	void writeString(const String& s);

	void writeData(const void* data, int len, int width);
	void writeImage(cairo_surface_t* img);

	ObjectOutputStream& operator <<(Serializeable* s);

	GString* getStr();

private:
	XOJ_TYPE_ATTRIB;

	ObjectEncoding* encoder;
};

#endif /* __OBJECTOUTPUTSTREAM_H__ */
