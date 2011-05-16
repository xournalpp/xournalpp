/*
 * Xournal++
 *
 * A background image of a page
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __BACKGROUNDIMAGE_H__
#define __BACKGROUNDIMAGE_H__

#include <XournalType.h>
#include <String.h>
#include <gtk/gtk.h>

class BackgroundImageContents;

class BackgroundImage {
public:
	BackgroundImage();
	BackgroundImage(const BackgroundImage & img);
	virtual ~BackgroundImage();

public:
	String getFilename();
	void loadFile(String filename, GError ** error);

	void setAttach(bool attach);

	void operator =(BackgroundImage & img);
	bool operator == (const BackgroundImage & img);

	void free();

	void clearSaveState();
	int getCloneId();
	void setCloneId(int id);

	void setFilename(String filename);

	bool isAttached();

	GdkPixbuf * getPixbuf();

private:
	XOJ_TYPE_ATTRIB;

	BackgroundImageContents * img;
};

#endif /* __BACKGROUNDIMAGE_H__ */
