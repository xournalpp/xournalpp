/*
 * Xournal++
 *
 * Hex encoded serialized stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ObjectEncoding.h"

class HexObjectEncoding : public ObjectEncoding
{
public:
	HexObjectEncoding();
	virtual ~HexObjectEncoding();

public:
	virtual void addData(const void* data, int len);

private:
	};
