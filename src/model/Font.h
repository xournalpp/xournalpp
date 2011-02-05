/*
 * Xournal++
 *
 * A font with a name and a size
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XFONT_H__
#define __XFONT_H__
#include "../util/String.h"

#include <gtk/gtk.h>
#include "../util/Serializeable.h"

class XojFont: public Serializeable {
public:
	XojFont();
	virtual ~XojFont();

	String getName();
	void setName(String name);

	double getSize();
	void setSize(double size);

	void operator =(const XojFont & font);

public:
	// Serialize interface
	void serialize(ObjectOutputStream & out);
	void readSerialized(ObjectInputStream & in) throw (InputStreamException);

private:
	void updateFontDesc();

private:
	String name;
	double size;
};

#endif /* __XFONT_H__ */
