/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <OutputStream.h>
#include <XournalType.h>

class Attribute
{
public:
	Attribute(const char* name);
	virtual ~Attribute();

public:
	virtual void writeOut(OutputStream* out) = 0;

	const char* getName();

private:
	XOJ_TYPE_ATTRIB;


	char* name;
};
