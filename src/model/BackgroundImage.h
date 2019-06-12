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
	BackgroundImage(BackgroundImage&& img) noexcept;
	virtual ~BackgroundImage();

	BackgroundImage& operator=(const BackgroundImage& img) = default;
	BackgroundImage& operator=(BackgroundImage&& img) = default;

public:
	string getFilename();
	void loadFile(string filename, GError** error);

	void loadFile(GInputStream* stream, string filename, GError** error);

	void setAttach(bool attach);
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
	std::shared_ptr<BackgroundImageContents> img;
};
