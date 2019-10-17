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

#include "XournalType.h"

#include <gtk/gtk.h>

#include <memory>

struct BackgroundImage
{
	BackgroundImage();
	BackgroundImage(const BackgroundImage& img);
	BackgroundImage(BackgroundImage&& img) noexcept;
	~BackgroundImage();

	BackgroundImage& operator=(const BackgroundImage& img) = default;
	BackgroundImage& operator=(BackgroundImage&& img) = default;


	bool operator==(const BackgroundImage& img);

	void free();

	void loadFile(string filename, GError** error);
	void loadFile(GInputStream* stream, string filename, GError** error);

	int getCloneId();
	void setCloneId(int id);
	void clearSaveState();

	string getFilename();
	void setFilename(string filename);

	bool isAttached();
	void setAttach(bool attach);

	GdkPixbuf* getPixbuf();

	bool isEmpty();

private:
	struct Content;
	std::shared_ptr<Content> img;
};
