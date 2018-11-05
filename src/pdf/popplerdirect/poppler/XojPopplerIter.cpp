#include "XojPopplerIter.h"

#include <poppler/Outline.h>
#include <poppler/GlobalParams.h>
#include <poppler/UnicodeMap.h>
#include "XojPopplerDocument.h"

XojPopplerIter::XojPopplerIter(XojPopplerDocument doc, const GooList* items)
{
    XOJ_INIT_TYPE(XojPopplerIter);

    this->doc = doc;
    this->items = items;
    this->index = 0;
}

XojPopplerIter::~XojPopplerIter()
{
    XOJ_RELEASE_TYPE(XojPopplerIter);
}

bool XojPopplerIter::next()
{
    XOJ_CHECK_TYPE(XojPopplerIter);

    this->index++;
    if (this->index >= this->items->getLength())
    {
        return false;
    }

    return true;
}

bool XojPopplerIter::isOpen()
{
    XOJ_CHECK_TYPE(XojPopplerIter);

    OutlineItem* item = (OutlineItem*) this->items->get(this->index);
    return item->isOpen();
}

XojPopplerIter* XojPopplerIter::getChildIter()
{
    XOJ_CHECK_TYPE(XojPopplerIter);

    OutlineItem* item = (OutlineItem*) this->items->get(this->index);
    item->open();
    if (!(item->hasKids() && item->getKids()))
    {
        return NULL;
    }

    XojPopplerIter* child = new XojPopplerIter(doc, item->getKids());

    return child;
}

string XojPopplerIter::unicodeToChar(const Unicode* unicode, int len)
{
    static UnicodeMap* uMap = NULL;
    if (uMap == NULL)
    {
        GooString* enc = new GooString("UTF-8");
        uMap = globalParams->getUnicodeMap(enc);
        uMap->incRefCnt();
        delete enc;
    }

    GooString gstr;
    gchar buf[8]; /* 8 is enough for mapping an unicode char to a string */
    for (int i = 0; i < len; ++i)
    {
        int n = uMap->mapUnicode(unicode[i], buf, sizeof(buf));
        gstr.append(buf, n);
    }

    return string(gstr.getCString());
}

XojPopplerAction* XojPopplerIter::getAction()
{
    XOJ_CHECK_TYPE(XojPopplerIter);

    OutlineItem* item = (OutlineItem*) this->items->get(this->index);
    if (item == NULL)
    {
        return NULL;
    }
    const LinkAction* linkAction = item->getAction();

    string title = unicodeToChar(item->getTitle(), item->getTitleLength());

    return new XojPopplerAction(this->doc, linkAction, title);
}
