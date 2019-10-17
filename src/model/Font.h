/*
 * Xournal++
 *
 * A font with a name and a size
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <serializing/Serializeable.h>
#include <XournalType.h>

#include <gtk/gtk.h>

class XojFont : public Serializeable
{
public:
	XojFont();
	virtual ~XojFont();

public:
	string getName();
	void setName(string name);

	double getSize();
	void setSize(double size);

	void operator=(const XojFont& font);

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in);

private:
	void updateFontDesc();

private:
	string name;
	double size = 0;
};
