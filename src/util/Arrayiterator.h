/*
 * Xournal++
 *
 * An iterator over an array
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <XournalType.h>

template<class T>
class ArrayIterator
{
public:
	ArrayIterator(const T* data, const int count)
	{
		this->data = data;
		this->i = 0;
		this->count = count;
	}

	virtual ~ArrayIterator() { }

	bool hasNext() const
	{
		return i < count;
	}

	T next()
	{
		return data[i++];
	}

	T get() const
	{
		return data[i];
	}

private:
	XOJ_TYPE_ATTRIB;

	int i;
	int count;
	const T* data;
};
