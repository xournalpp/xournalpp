#include "XojPopplerAction.h"
#include "XojPopplerDocument.h"

XojPopplerAction::XojPopplerAction(XojPopplerDocument doc, const LinkAction* linkAction, string title) : doc(doc), title(title)
{
    XOJ_INIT_TYPE(XojPopplerAction);

    this->linkAction = linkAction;
}

XojPopplerAction::~XojPopplerAction()
{
    XOJ_RELEASE_TYPE(XojPopplerAction);
}

void XojPopplerAction::linkFromDest(LinkDestination* link, const LinkDest* dest)
{
    XOJ_CHECK_TYPE(XojPopplerAction);

    int pageNum = 0;
    if (dest->isPageRef())
    {
        Ref pageRef = dest->getPageRef();
        pageNum = doc.getDoc()->findPage(pageRef.num, pageRef.gen);
    }
    else
    {
        pageNum = dest->getPageNum();
    }
    pageNum--;

    switch (dest->getKind())
    {
    case destXYZ:
    {
        XojPopplerPage* popplerPage = doc.getPage(MAX(0, pageNum));
        if (!popplerPage)
        {
            return;
        }
        double height = popplerPage->getHeight();

        if (dest->getChangeLeft())
        {
            link->setChangeLeft(dest->getLeft());
        }

        if (dest->getChangeTop())
        {
            link->setChangeTop(height - MIN(height, dest->getTop()));
        }

        if (dest->getChangeZoom())
        {
            link->setChangeZoom(dest->getZoom());
        }

        break;
    }
    case destFitB:
    case destFit:
        //ev_dest = ev_link_dest_new_fit(dest->page_num - 1);
        break;
    case destFitBH:
    case destFitH:
    {
        //PopplerPage *popplerPage = getPage(MAX (0, dest->page_num - 1));
        //if (!popplerPage) {
        //	return;
        //}
        //double height;
        //poppler_page_get_size(popplerPage, NULL, &height);
        //ev_dest = ev_link_dest_new_fith(dest->page_num - 1, height - MIN (height, dest->top), dest->change_top);
        break;
    }
    case destFitBV:
    case destFitV:
        //ev_dest = ev_link_dest_new_fitv(dest->page_num - 1, dest->left, dest->change_left);
        break;
    case destFitR:
    {
        //PopplerPage *poppler_page;
        //double height;
        //
        //poppler_page = poppler_document_get_page(pdf_document->document, MAX (0, dest->page_num - 1));
        //poppler_page_get_size(poppler_page, NULL, &height);
        //ev_dest = ev_link_dest_new_fitr(dest->page_num - 1, dest->left, height - MIN (height, dest->bottom),
        //		dest->right, height - MIN (height, dest->top));
        //g_object_unref(poppler_page);
        break;
    }
    }

    link->setPdfPage(pageNum);
}

XojLinkDest* XojPopplerAction::getDestination()
{
    XOJ_CHECK_TYPE(XojPopplerAction);

    XojLinkDest* dest = link_dest_new();
    dest->dest = new LinkDestination();
    dest->dest->setName(this->title);

    if (this->linkAction == NULL)
    {
        return dest;
    }

    // every other action is not supported in Xournal
    if (this->linkAction->getKind() == actionGoTo)
    {
        const LinkGoTo* link = dynamic_cast<const LinkGoTo*> (this->linkAction);

        const GooString* namedDest = link->getNamedDest();
        const LinkDest* d = NULL;
        if (namedDest)
        {
            d = doc.getDoc()->findDest(namedDest);
        }

        if (!d)
        {
            d = link->getDest();
        }

        if (d)
        {
            linkFromDest(dest->dest, d);
        }
    }

    return dest;
}

string XojPopplerAction::getTitle()
{
    XOJ_CHECK_TYPE(XojPopplerAction);

    return this->title;
}
