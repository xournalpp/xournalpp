/*
 * Xournal++
 *
 * An iterator over an array
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XournalType.h"

template <class T>
class ArrayIterator
{
public:
	ArrayIterator(const T* data, const int count)
	{
		XOJ_INIT_TYPE(ArrayIterator);

		this->data = data;
		this->i = 0;
		this->count = count;
	}

	virtual ~ArrayIterator()
	{
		XOJ_RELEASE_TYPE(ArrayIterator);
	}

	bool hasNext() const
	{
		XOJ_CHECK_TYPE(ArrayIterator);

		return i < count;
	}

	T next()
	{
		XOJ_CHECK_TYPE(ArrayIterator);

		return data[i++];
	}

	T get() const
	{
		XOJ_CHECK_TYPE(ArrayIterator);

		return data[i];
	}

private:
	XOJ_TYPE_ATTRIB;

	int i;
	int count;
	const T* data;
};
