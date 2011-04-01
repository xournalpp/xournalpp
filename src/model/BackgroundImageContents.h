/*
 * Xournal++
 *
 * The contents of a background image
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __BACKGROUNDIMAGECONTENTS_H__
#define __BACKGROUNDIMAGECONTENTS_H__

#include <glib.h>
#include "../util/String.h"
#include "../util/XournalType.h"

class BackgroundImageContents {
public:
	BackgroundImageContents(String filename, GError ** error);

private:
	BackgroundImageContents();
	BackgroundImageContents(const BackgroundImageContents & contents);
	void operator =(const BackgroundImageContents & contents);

private:
	virtual ~BackgroundImageContents();

public:
	void unreference();
	void reference();

public:
	String getFilename();
	void setFilename(String filename);

	bool isAttach();
	void setAttach(bool attach);

	int getPageId();
	void setPageId(int id);

	GdkPixbuf * getPixbuf();

private:
	XOJ_TYPE_ATTRIB;

	int ref;
	String filename;
	bool attach;
	int pageId;
	GdkPixbuf * pixbuf;
};

#endif /* __BACKGROUNDIMAGECONTENTS_H__ */
