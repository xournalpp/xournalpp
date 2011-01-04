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
#include "String.h"

#include <gtk/gtk.h>


class XFont {
public:
	XFont();
	virtual ~XFont();

	String getName();
	void setName(String name);

	bool isItalic();
	bool isBold();

	void setItalic(bool italic);
	void setBold(bool bold);

	double getSize();
	void setSize(double size);
private:
	void updateFontDesc();


private:
	String name;
	double size;

	bool italic;
	bool bold;
};

#endif /* __XFONT_H__ */
