#include "PdfBookmark.h"
#include "../../model/LinkDestination.h"

class Bookmark {
public:
	Bookmark(String name, int level, int page, int top) {
		this->name = name;
		this->level = level;
		this->parent = -1;
		this->last = -1;
		this->first = -1;
		this->prev = -1;
		this->next = -1;
		this->page = page;
		this->top = top;
	}

	String name;
	int level;

	int parent;
	int last;
	int first;
	int next;
	int prev;
	int page;
	int top;
};

PdfBookmarks::PdfBookmarks() {
	XOJ_INIT_TYPE(PdfBookmarks);
}

PdfBookmarks::~PdfBookmarks() {
	XOJ_RELEASE_TYPE(PdfBookmarks);
}

void PdfBookmarks::createBookmarks(GtkTreeModel * model, GList * &data, GtkTreeIter * iter, int level, Document * doc) {
	XOJ_CHECK_TYPE(PdfBookmarks);

	XojLinkDest * link = NULL;
	LinkDestination * dest = NULL;

	do {
		gtk_tree_model_get(model, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);
		dest = link->dest;

		g_object_unref(link);

		int page = doc->findPdfPage(dest->getPdfPage());
		if (page == -1) {
			continue;
		}

		data = g_list_append(data, new Bookmark(dest->getName(), level, page, dest->getTop()));

		GtkTreeIter children = { 0 };

		if (gtk_tree_model_iter_children(model, &children, iter)) {
			createBookmarks(model, data, &children, level + 1, doc);
		}
	} while (gtk_tree_model_iter_next(model, iter));
}

GList * PdfBookmarks::exportBookmarksFromTreeModel(GtkTreeModel * model, Document * doc) {
	XOJ_CHECK_TYPE(PdfBookmarks);

	GList * data = NULL;
	GtkTreeIter iter = { 0 };

	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		return false;
	}

	createBookmarks(model, data, &iter, 0, doc);

	return data;
}

void PdfBookmarks::writeOutlines(Document * doc, PdfWriter * writer, int * outlineRoot, GList * pageIds) {
	XOJ_CHECK_TYPE(PdfBookmarks);

	GtkTreeModel * model = doc->getContentsModel();
	if (!model) {
		return;
	}

	GList * bookmarkList = exportBookmarksFromTreeModel(model, doc);
	int bookmarksLenght = g_list_length(bookmarkList);

	if (bookmarksLenght == 0) {
		return;
	}

	Bookmark ** bookmarks = new Bookmark *[bookmarksLenght];

	int maxLevel = 0;
	int i = 0;
	for (GList * l = bookmarkList; l != NULL; l = l->next) {
		Bookmark * b = (Bookmark *) l->data;
		if (maxLevel < b->level) {
			maxLevel = b->level;
		}
		bookmarks[i++] = b;
	}

	int * levels = new int[maxLevel + 1];
	for (int u = 0; u < maxLevel + 1; u++) {
		levels[u] = 0;
	}

	int level = 0;
	for (i = 0; i < bookmarksLenght; i++) {
		Bookmark * b = bookmarks[i];
		if (b->level > 0) {
			int parent = levels[b->level - 1];
			//Set parent and last pointers
			b->parent = parent;
			bookmarks[parent]->last = i;
			if (b->level > level) {
				//Level increasing: set first pointer
				bookmarks[parent]->first = i;
			}
		} else {
			bookmarks[i]->parent = bookmarksLenght;
		}

		if (b->level <= level && i > 0) {
			//Set prev and next pointers
			int prev = levels[b->level];
			bookmarks[prev]->next = i;
			bookmarks[i]->prev = prev;
		}
		levels[b->level] = i;
		level = b->level;
	}

	int n = writer->getObjectId();

	for (i = 0; i < bookmarksLenght; i++) {
		Bookmark * b = bookmarks[i];
		writer->writeObj();

		writer->write("<<\n/Title ");
		writer->writeTxt(b->name.c_str());
		writer->write("\n");

		writer->write("/Parent ");
		writer->write(n + b->parent);
		writer->write(" 0 R\n");

		if (b->prev != -1) {
			writer->write("/Prev ");
			writer->write(n + b->prev);
			writer->write(" 0 R\n");
		}

		if (b->next != -1) {
			writer->write("/Next ");
			writer->write(n + b->next);
			writer->write(" 0 R\n");
		}

		if (b->first != -1) {
			writer->write("/First ");
			writer->write(n + b->first);
			writer->write(" 0 R\n");
		}

		if (b->last != -1) {
			writer->write("/Last ");
			writer->write(n + b->last);
			writer->write(" 0 R\n");
		}

		PageRef p = doc->getPage(b->page);
		float top = 0;
		if (p.isValid()) {
			top = (p.getHeight() - b->top) * 72;
		}

		//Outline items
		char buffer[256];

		int pObjId = 0;
		int * pObjIdPtr = (int *)g_list_nth_data(pageIds, b->page);
		if (pObjIdPtr) {
			pObjId = *pObjIdPtr;
		}

		sprintf(buffer, "/Dest [%d 0 R /XYZ 0 %.2f null]\n", pObjId, top /*($this->h-$o['y'])*$this->k*/);

		writer->write(buffer);
		writer->write("/Count 0\n>>\n");
		writer->write("endobj\n");
	}

	//Outline root
	writer->writeObj();
	*outlineRoot = writer->getObjectId() - 1;
	writer->write("<<\n/Type /Outlines /First ");
	writer->write(n);
	writer->write(" 0 R\n");
	writer->write("/Last ");
	writer->write(n + levels[0]);
	writer->write(" 0 R\n>>\n");
	writer->write("endobj\n");

	for (i = 0; i < bookmarksLenght; i++) {
		delete bookmarks[i];
	}

	delete[] bookmarks;
	delete[] levels;

	return;
}
