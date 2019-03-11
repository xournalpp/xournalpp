/*
 * Xournal++
 *
 * The contents of a background image
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

class BackgroundImageContents
{
public:
	BackgroundImageContents(string filename, GError** error);
	BackgroundImageContents(GInputStream* stream, string filename, GError** error);

private:
	BackgroundImageContents();
	BackgroundImageContents(const BackgroundImageContents& contents);
	void operator=(const BackgroundImageContents& contents);

private:
	virtual ~BackgroundImageContents();

public:
	void unreference();
	void reference();

public:
	string getFilename();
	void setFilename(string filename);

	bool isAttach();
	void setAttach(bool attach);

	int getPageId();
	void setPageId(int id);

	GdkPixbuf* getPixbuf();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Reference counter
	 */
	int ref = 1;

	string filename;
	bool attach = false;

	/**
	 *
	 */
	int pageId = -1;
	GdkPixbuf* pixbuf = NULL;
};
