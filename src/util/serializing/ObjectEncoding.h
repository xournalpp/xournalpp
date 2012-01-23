/*
 * Xournal++
 *
 * Encoding for serialized streams
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __OBJECTENCODING_H__
#define __OBJECTENCODING_H__

#include <XournalType.h>

#include <glib.h>

class ObjectEncoding {
public:
	ObjectEncoding();
	virtual ~ObjectEncoding();

public:
	void addStr(const char * str);
	virtual void addData(const void * data, int len) = 0;

	GString * getData();

public:
	XOJ_TYPE_ATTRIB;

	GString * data;
};

#endif /* __OBJECTENCODING_H__ */

