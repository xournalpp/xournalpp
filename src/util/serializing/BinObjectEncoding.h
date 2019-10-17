/*
 * Xournal++
 *
 * Binary encoded serialized stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ObjectEncoding.h"

class BinObjectEncoding : public ObjectEncoding
{
public:
	BinObjectEncoding();
	virtual ~BinObjectEncoding();

public:
	virtual void addData(const void* data, int len);

private:
	};
