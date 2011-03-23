/*
 * Xournal++
 *
 * An iterator over a GList
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __LISTITERATOR_H__
#define __LISTITERATOR_H__

#include <gtk/gtk.h>

template<class T>
class ListIterator {
public:
	ListIterator(GList * data, bool reverse = false) {
		if (reverse) {
			this->data = g_list_last(data);
		} else {
			this->data = data;
		}
		this->reverse = reverse;
		this->copied = false;
		this->list = data;
	}

	virtual ~ListIterator() {
		if (this->copied) {
			g_list_free(this->list);
		}
		this->copied = false;
		this->data = NULL;
	}

	/**
	 * If the source changes while you are using the iterator nothing happens
	 */
	void freeze() {
		if (this->copied) {
			return;
		}
		this->copied = true;
		this->list = g_list_copy(this->data);
		this->data = this->list;
	}

	bool hasNext() {
		return this->data != NULL;
	}

	T next() {
		T d = (T) this->data->data;
		if (reverse) {
			this->data = this->data->prev;
		} else {
			this->data = this->data->next;
		}
		return d;
	}

	int getLength() {
		return g_list_length(this->list);
	}

	void reset() {
		if (this->reverse) {
			this->data = g_list_last(this->list);
		} else {
			this->data = this->list;
		}
	}

private:
	GList * list;
	GList * data;
	bool reverse;
	bool copied;
};

#endif /* __LISTITERATOR_H__ */
