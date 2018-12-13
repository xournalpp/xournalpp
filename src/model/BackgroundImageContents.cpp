#include "BackgroundImageContents.h"
#include <Util.h>

BackgroundImageContents::BackgroundImageContents(path filename, GError** error)
{
	XOJ_INIT_TYPE(BackgroundImageContents);

	this->filename = filename;
	this->ref = 1;
	this->pageId = -1;
	this->attach = false;
	this->pixbuf = gdk_pixbuf_new_from_file(PATH_TO_CSTR(filename), error);
}

BackgroundImageContents::~BackgroundImageContents()
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	g_object_unref(this->pixbuf);

	XOJ_RELEASE_TYPE(BackgroundImageContents);
}

void BackgroundImageContents::unreference()
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	this->ref--;
	if (this->ref < 0)
	{
		delete this;
	}
}

void BackgroundImageContents::reference()
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	this->ref++;
}

path BackgroundImageContents::getFilename()
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	return this->filename;
}

void BackgroundImageContents::setFilename(path filename)
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	this->filename = filename;
}

bool BackgroundImageContents::isAttach()
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	return this->attach;
}

void BackgroundImageContents::setAttach(bool attach)
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	this->attach = attach;
}

int BackgroundImageContents::getPageId()
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	return this->pageId;
}

void BackgroundImageContents::setPageId(int id)
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	this->pageId = id;
}

GdkPixbuf* BackgroundImageContents::getPixbuf()
{
	XOJ_CHECK_TYPE(BackgroundImageContents);

	return this->pixbuf;
}
