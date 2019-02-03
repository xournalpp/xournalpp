#include "PopplerGlibPageBookmarkIterator.h"

PopplerGlibPageBookmarkIterator::PopplerGlibPageBookmarkIterator(PopplerIndexIter* iter, PopplerDocument* document)
 : iter(iter),
   document(document)
{
	XOJ_INIT_TYPE(PopplerGlibPageBookmarkIterator);
	g_object_ref(document);
}

PopplerGlibPageBookmarkIterator::~PopplerGlibPageBookmarkIterator()
{
	XOJ_CHECK_TYPE(PopplerGlibPageBookmarkIterator);

	poppler_index_iter_free(iter);
	iter = NULL;

	if (document)
	{
		g_object_unref(document);
		document = NULL;
	}

	XOJ_RELEASE_TYPE(PopplerGlibPageBookmarkIterator);
}

bool PopplerGlibPageBookmarkIterator::next()
{
	XOJ_CHECK_TYPE(PopplerGlibPageBookmarkIterator);

	return poppler_index_iter_next(iter);
}

bool PopplerGlibPageBookmarkIterator::isOpen()
{
	XOJ_CHECK_TYPE(PopplerGlibPageBookmarkIterator);

	return poppler_index_iter_is_open(iter);
}

XojPdfBookmarkIterator* PopplerGlibPageBookmarkIterator::getChildIter()
{
	XOJ_CHECK_TYPE(PopplerGlibPageBookmarkIterator);

	PopplerIndexIter* child = poppler_index_iter_get_child(iter);
	if (child == NULL)
	{
		return NULL;
	}

	return new PopplerGlibPageBookmarkIterator(child, document);
}

XojPdfAction* PopplerGlibPageBookmarkIterator::getAction()
{
	XOJ_CHECK_TYPE(PopplerGlibPageBookmarkIterator);

	PopplerAction* action = poppler_index_iter_get_action(iter);

	if (action == NULL)
	{
		return NULL;
	}

	return new PopplerGlibAction(action, document);
}
