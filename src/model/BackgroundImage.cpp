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

struct BackgroundImage::Content
{
	Content(string filename, GError** error)
			: filename(std::move(filename)), pixbuf(gdk_pixbuf_new_from_file(this->filename.c_str(), error))
	{
	}

	Content(GInputStream* stream, string filename, GError** error)
			: filename(std::move(filename)), pixbuf(gdk_pixbuf_new_from_stream(stream, nullptr, error))
	{
	}

	~Content()
	{
		g_object_unref(this->pixbuf);
		this->pixbuf = nullptr;
	};

	Content(const Content&) = delete;
	Content(Content&&) = default;
	Content& operator=(const Content&) = delete;
	Content& operator=(Content&&) = default;

	string filename;
	GdkPixbuf* pixbuf = nullptr;
	int pageId = -1;
	bool attach = false;
};

BackgroundImage::BackgroundImage()
{
}

BackgroundImage::BackgroundImage(const BackgroundImage& img) : img(img.img)
{
}

BackgroundImage::BackgroundImage(BackgroundImage&& img) noexcept : img(std::move(img.img))
{
}

BackgroundImage::~BackgroundImage()
{
}

bool BackgroundImage::operator==(const BackgroundImage& img)
{
	return this->img == img.img;
}

void BackgroundImage::free()
{
	this->img.reset();
}

void BackgroundImage::loadFile(string filename, GError** error)
{
	this->img = std::make_shared<Content>(std::move(filename), error);
}

void BackgroundImage::loadFile(GInputStream* stream, string filename, GError** error)
{
	this->img = std::make_shared<Content>(stream, std::move(filename), error);
}

int BackgroundImage::getCloneId()
{
	return this->img ? this->img->pageId : -1;
}

void BackgroundImage::setCloneId(int id)
{
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
	return this->img ? this->img->filename : "";
}

void BackgroundImage::setFilename(string filename)
{
	if (this->img)
	{
		this->img->filename = std::move(filename);
	}
}

bool BackgroundImage::isAttached()
{
	return this->img ? this->img->attach : false;
}

void BackgroundImage::setAttach(bool attach)
{
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
	return this->img ? this->img->pixbuf : nullptr;
}

bool BackgroundImage::isEmpty()
{
	return !this->img;
}
