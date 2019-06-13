#include "BackgroundImage.h"

#include "Stacktrace.h"

/*
 * The contents of a background image
 *
 * Internal impl object, dont move this to an external header/source file due this is the best way to reduce code
 * bloat and increase encapsulation. This object is only used in this source scope and is a RAII Container for the
 * GdkPixbuf*
 * No xournal memory leak tests necessary, because we use smart ptrs to ensure memory correctness
 */

struct BackgroundImageContents
{
	BackgroundImageContents(string filename, GError** error)
			: filename(std::move(filename)), pixbuf(gdk_pixbuf_new_from_file(this->filename.c_str(), error))
	{
	}

	BackgroundImageContents(GInputStream* stream, string filename, GError** error)
			: filename(std::move(filename)), pixbuf(gdk_pixbuf_new_from_stream(stream, nullptr, error))
	{
	}

	~BackgroundImageContents()
	{
		g_object_unref(this->pixbuf);
		this->pixbuf = nullptr;
	};

	BackgroundImageContents(const BackgroundImageContents&) = delete;
	BackgroundImageContents(BackgroundImageContents&&) = default;
	BackgroundImageContents& operator=(const BackgroundImageContents&) = delete;
	BackgroundImageContents& operator=(BackgroundImageContents&&) = default;

	string filename;
	GdkPixbuf* pixbuf = nullptr;
	int pageId = -1;
	bool attach = false;
};

BackgroundImage::BackgroundImage()
{
	XOJ_INIT_TYPE(BackgroundImage);
}

BackgroundImage::BackgroundImage(const BackgroundImage& img) : img(img.img)
{
	XOJ_INIT_TYPE(BackgroundImage);
	XOJ_CHECK_TYPE_OBJ((&img), BackgroundImage);
}

BackgroundImage::BackgroundImage(BackgroundImage&& img) noexcept : img(std::move(img.img))
{
	XOJ_INIT_TYPE(BackgroundImage);
	XOJ_CHECK_TYPE_OBJ((&img), BackgroundImage);
}

BackgroundImage::~BackgroundImage()
{
	XOJ_CHECK_TYPE(BackgroundImage);
	XOJ_RELEASE_TYPE(BackgroundImage);
}

bool BackgroundImage::operator==(const BackgroundImage& img)
{
	XOJ_CHECK_TYPE(BackgroundImage);
	return this->img == img.img;
}

void BackgroundImage::loadFile(string filename, GError** error)
{
	XOJ_CHECK_TYPE(BackgroundImage);
	this->img = std::make_shared<BackgroundImageContents>(std::move(filename), error);
}

void BackgroundImage::loadFile(GInputStream* stream, string filename, GError** error)
{
	XOJ_CHECK_TYPE(BackgroundImage);
	this->img = std::make_shared<BackgroundImageContents>(stream, std::move(filename), error);
}

int BackgroundImage::getCloneId()
{
	XOJ_CHECK_TYPE(BackgroundImage);
	return this->img ? this->img->pageId : -1;
}

void BackgroundImage::setCloneId(int id)
{
	XOJ_CHECK_TYPE(BackgroundImage);
	if (this->img)
	{
		this->img->pageId = id;
	}
}

void BackgroundImage::clearSaveState()
{
	this->setCloneId(-1);
}

string BackgroundImage::getFilename()
{
	XOJ_CHECK_TYPE(BackgroundImage);
	return this->img ? this->img->filename : "";
}

void BackgroundImage::setFilename(string filename)
{
	XOJ_CHECK_TYPE(BackgroundImage);
	if (this->img)
	{
		this->img->filename = std::move(filename);
	}
}

bool BackgroundImage::isAttached()
{
	XOJ_CHECK_TYPE(BackgroundImage);
	return this->img ? this->img->attach : false;
}

void BackgroundImage::setAttach(bool attach)
{
	XOJ_CHECK_TYPE(BackgroundImage);
	if (!this->img)
	{
		g_warning("BackgroundImage::setAttach: please load first an image before call setAttach!");
		Stacktrace::printStracktrace();
		return;
	}
	this->img->attach = attach;
}

GdkPixbuf* BackgroundImage::getPixbuf()
{
	XOJ_CHECK_TYPE(BackgroundImage);
	return this->img ? this->img->pixbuf : nullptr;
}

bool BackgroundImage::isEmpty()
{
	XOJ_CHECK_TYPE(BackgroundImage);
	return !this->img;
}

void BackgroundImage::free()
{
	XOJ_CHECK_TYPE(BackgroundImage);
	this->img.reset();
}
