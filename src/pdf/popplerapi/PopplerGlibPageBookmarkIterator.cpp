#include "PopplerGlibPageBookmarkIterator.h"

PopplerGlibPageBookmarkIterator::PopplerGlibPageBookmarkIterator(PopplerIndexIter* iter, PopplerDocument* document)
 : iter(iter),
   document(document)
{
	g_object_ref(document);
}

PopplerGlibPageBookmarkIterator::~PopplerGlibPageBookmarkIterator()
{
	poppler_index_iter_free(iter);
	iter = nullptr;

	if (document)
	{
		g_object_unref(document);
		document = nullptr;
	}
}

bool PopplerGlibPageBookmarkIterator::next()
{
	return poppler_index_iter_next(iter);
}

bool PopplerGlibPageBookmarkIterator::isOpen()
{
	return poppler_index_iter_is_open(iter);
}

XojPdfBookmarkIterator* PopplerGlibPageBookmarkIterator::getChildIter()
{
	PopplerIndexIter* child = poppler_index_iter_get_child(iter);
	if (child == nullptr)
	{
		return nullptr;
	}

	return new PopplerGlibPageBookmarkIterator(child, document);
}

XojPdfAction* PopplerGlibPageBookmarkIterator::getAction()
{
	PopplerAction* action = poppler_index_iter_get_action(iter);

	if (action == nullptr)
	{
		return nullptr;
	}

	return new PopplerGlibAction(action, document);
}
