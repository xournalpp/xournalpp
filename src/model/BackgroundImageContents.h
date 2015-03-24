/*
 * Xournal++
 *
 * The contents of a background image
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __BACKGROUNDIMAGECONTENTS_H__
#define __BACKGROUNDIMAGECONTENTS_H__

#include <glib.h>
#include <StringUtils.h>
#include <XournalType.h>
#include <gtk/gtk.h>
#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class BackgroundImageContents
{
public:
    BackgroundImageContents(path filename, GError** error);

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
    path getFilename();
    void setFilename(path filename);

    bool isAttach();
    void setAttach(bool attach);

    int getPageId();
    void setPageId(int id);

    GdkPixbuf* getPixbuf();

private:
    XOJ_TYPE_ATTRIB;

    int ref;
    path filename;
    bool attach;
    int pageId;
    GdkPixbuf* pixbuf;
};

#endif /* __BACKGROUNDIMAGECONTENTS_H__ */
