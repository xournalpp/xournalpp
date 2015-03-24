/*
 * Xournal++
 *
 * An iterator over a GList
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __LISTITERATOR_H__
#define __LISTITERATOR_H__

#include <gtk/gtk.h>
#include <XournalType.h>

template<class T>
class ListIterator
{
public:
	ListIterator(GList* data, bool reverse = false)
	{
		XOJ_INIT_TYPE(ListIterator);

		if (reverse)
		{
			this->data = g_list_last(data);
		}
		else
		{
			this->data = data;
		}
		this->reverse = reverse;
		this->copied = false;
		this->list = data;
	}

	ListIterator(const ListIterator& it)
	{
		XOJ_INIT_TYPE(ListIterator);

		*this = it;
		if (this->copied)
		{
			this->list = g_list_copy(this->data);
			this->data = this->list;
		}
	}

	virtual ~ListIterator()
	{
		XOJ_CHECK_TYPE(ListIterator);

		if (this->copied)
		{
			g_list_free(this->list);
		}
		this->copied = false;
		this->data = NULL;

		XOJ_RELEASE_TYPE(ListIterator);
	}

	/**
	 * If the source changes while you are using the iterator nothing happens
	 */
	void freeze()
	{
		XOJ_CHECK_TYPE(ListIterator);

		if (this->copied)
		{
			return;
		}
		this->copied = true;
		this->list = g_list_copy(this->data);
		this->data = this->list;
	}

	bool hasNext()
	{
		XOJ_CHECK_TYPE(ListIterator);

		return this->data != NULL;
	}

	T next()
	{
		XOJ_CHECK_TYPE(ListIterator);

		T d = (T) this->data->data;
		if (reverse)
		{
			this->data = this->data->prev;
		}
		else
		{
			this->data = this->data->next;
		}
		return d;
	}

	int getLength()
	{
		XOJ_CHECK_TYPE(ListIterator);

		return g_list_length(this->list);
	}

	void reset()
	{
		XOJ_CHECK_TYPE(ListIterator);

		if (this->reverse)
		{
			this->data = g_list_last(this->list);
		}
		else
		{
			this->data = this->list;
		}
	}

private:
	XOJ_TYPE_ATTRIB;

	GList* list;
	GList* data;
	bool reverse;
	bool copied;
};

#endif /* __LISTITERATOR_H__ */
