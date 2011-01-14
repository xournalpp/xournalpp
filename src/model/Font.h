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


class XojFont {
public:
	XojFont();
	virtual ~XojFont();

	String getName();
	void setName(String name);

	double getSize();
	void setSize(double size);

	void operator =(const XojFont & font);
private:
	void updateFontDesc();


private:
	String name;
	double size;
};

#endif /* __XFONT_H__ */
