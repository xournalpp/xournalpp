/*
 * Xournal++
 *
 * A background image of a page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

class BackgroundImageContents;

class BackgroundImage
{
public:
	BackgroundImage();
	BackgroundImage(const BackgroundImage& img);
	virtual ~BackgroundImage();

public:
	string getFilename();
	void loadFile(string filename, GError** error);

	void setAttach(bool attach);

	void operator=(BackgroundImage& img);
	bool operator==(const BackgroundImage& img);

	void free();

	void clearSaveState();
	int getCloneId();
	void setCloneId(int id);

	void setFilename(string filename);

	bool isAttached();
	bool isEmpty();

	GdkPixbuf* getPixbuf();

private:
	XOJ_TYPE_ATTRIB;

	BackgroundImageContents* img;
};
