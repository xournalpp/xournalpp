#include "PopplerGlibAction.h"

PopplerGlibAction::PopplerGlibAction(PopplerAction* action)
 : action(action)
{
    XOJ_INIT_TYPE(PopplerGlibAction);
}

PopplerGlibAction::~PopplerGlibAction()
{
	XOJ_CHECK_TYPE(PopplerGlibAction);

	poppler_action_free(action);
	action = NULL;

    XOJ_RELEASE_TYPE(PopplerGlibAction);
}

XojLinkDest* PopplerGlibAction::getDestination()
{
	XOJ_CHECK_TYPE(PopplerGlibAction);

	XojLinkDest* dest = link_dest_new();
	dest->dest = new LinkDestination();
	dest->dest->setName(getTitle());

	// every other action is not supported in Xournal
	if (action->type == POPPLER_ACTION_GOTO_DEST)
	{
		PopplerActionGotoDest* actionDest = (PopplerActionGotoDest*)action;
		PopplerDest* pDest = actionDest->dest;

		if (pDest == NULL)
		{
			return dest;
		}

		linkFromDest(dest->dest, pDest);
	}

	return dest;
}

void PopplerGlibAction::linkFromDest(LinkDestination* link, PopplerDest* pDest)
{
	XOJ_CHECK_TYPE(PopplerGlibAction);

	switch(pDest->type)
	{
	case POPPLER_DEST_UNKNOWN:
		break;
	case POPPLER_DEST_XYZ:
		{
//			int page_num;
//			double left;
//			double bottom;
//			double right;
//			double top;
//			double zoom;
//
//
//			XojPopplerPage* popplerPage = doc.getPage(MAX(0, pageNum));
//			if (!popplerPage)
//			{
//				return;
//			}
//			double height = popplerPage->getHeight();
//
//			if (dest->getChangeLeft())
//			{
//				link->setChangeLeft(dest->getLeft());
//			}
//
//			if (dest->getChangeTop())
//			{
//				link->setChangeTop(height - MIN(height, dest->getTop()));
//			}
//
//			if (dest->getChangeZoom())
//			{
//				link->setChangeZoom(dest->getZoom());
//			}
		}
		break;
	case POPPLER_DEST_FIT:
		break;
	case POPPLER_DEST_FITH:
		break;
	case POPPLER_DEST_FITV:
		break;
	case POPPLER_DEST_FITR:
		break;
	case POPPLER_DEST_FITB:
		break;
	case POPPLER_DEST_FITBH:
		break;
	case POPPLER_DEST_FITBV:
		break;
	case POPPLER_DEST_NAMED:
		break;

	default:
		break;
	}

	link->setPdfPage(pDest->page_num);
}

string PopplerGlibAction::getTitle()
{
	XOJ_CHECK_TYPE(PopplerGlibAction);

	return ((PopplerActionAny*)action)->title;
}
