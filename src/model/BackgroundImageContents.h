/*
 * Xournal++
 *
 * The contents of a background image
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __BACKGROUNDIMAGECONTENTS_H__
#define __BACKGROUNDIMAGECONTENTS_H__

#include <glib.h>
#include <StringUtils.h>
#include <XournalType.h>
#include <gtk/gtk.h>

class BackgroundImageContents
{
public:
    BackgroundImageContents(string filename, GError** error);

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

    int ref;
    string filename;
    bool attach;
    int pageId;
    GdkPixbuf* pixbuf;
};

#endif /* __BACKGROUNDIMAGECONTENTS_H__ */
