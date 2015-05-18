#include "BackgroundImage.h"

#include "BackgroundImageContents.h"
#include <Stacktrace.h>

BackgroundImage::BackgroundImage()
{
	XOJ_INIT_TYPE(BackgroundImage);

	this->img = NULL;
}

BackgroundImage::BackgroundImage(const BackgroundImage& img)
{
	XOJ_INIT_TYPE(BackgroundImage);
	XOJ_CHECK_TYPE_OBJ((&img), BackgroundImage);

	this->img = img.img;
	if (this->img)
	{
		this->img->reference();
	}
}

BackgroundImage::~BackgroundImage()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		this->img->unreference();
	}
	this->img = NULL;

	XOJ_RELEASE_TYPE(BackgroundImage);
}

path BackgroundImage::getFilename()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img != NULL)
	{
		return this->img->getFilename();
	}
	return path("");
}

void BackgroundImage::loadFile(path filename, GError** error)
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img != NULL)
	{
		this->img->unreference();
	}
	this->img = new BackgroundImageContents(filename, error);
}

void BackgroundImage::setAttach(bool attach)
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img != NULL)
	{
		this->img->setAttach(true);
	}
	else
	{
		g_warning("BackgroundImage::setAttach:please load first an image before call setAttach!");
		Stacktrace::printStracktrace();
	}
}

void BackgroundImage::operator=(BackgroundImage& img)
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		this->img->unreference();
	}
	this->img = img.img;
	if (this->img)
	{
		this->img->reference();
	}
}

bool BackgroundImage::operator==(const BackgroundImage& img)
{
	XOJ_CHECK_TYPE(BackgroundImage);

	return this->img == img.img;
}

void BackgroundImage::free()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		this->img->unreference();
	}
	this->img = NULL;
}

void BackgroundImage::clearSaveState()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		this->img->setPageId(-1);
	}
}

int BackgroundImage::getCloneId()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		return this->img->getPageId();
	}
	return -1;
}

void BackgroundImage::setCloneId(int id)
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		this->img->setPageId(id);
	}
}

void BackgroundImage::setFilename(path filename)
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		this->img->setFilename(filename);
	}
}

bool BackgroundImage::isAttached()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		return this->img->isAttach();
	}
	return false;
}

bool BackgroundImage::isEmpty()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	return this->img == NULL;
}

GdkPixbuf* BackgroundImage::getPixbuf()
{
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img)
	{
		return this->img->getPixbuf();
	}
	return NULL;
}
