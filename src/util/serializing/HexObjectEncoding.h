/*
 * Xournal++
 *
 * Hex encoded serialized stream
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __HEXOBJECTENCODING_H__
#define __HEXOBJECTENCODING_H__

#include "ObjectEncoding.h"

class HexObjectEncoding : public ObjectEncoding
{
public:
	HexObjectEncoding();
	virtual ~HexObjectEncoding();

public:
	virtual void addData(const void* data, int len);

private:
	XOJ_TYPE_ATTRIB;
};

#endif /* __HEXOBJECTENCODING_H__ */
