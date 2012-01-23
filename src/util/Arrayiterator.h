/*
 * Xournal++
 *
 * An iterator over an array
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ARRAYITERATOR_H__
#define __ARRAYITERATOR_H__

#include <XournalType.h>

template<class T>
class ArrayIterator {
public:
	ArrayIterator(const T * data, const int count) {
		this->data = data;
		this->i = 0;
		this->count = count;
	}

	virtual ~ArrayIterator() {
	}

	bool hasNext() const {
		return i < count;
	}

	T next() {
		return data[i++];
	}

	T get() const {
		return data[i];
	}

private:
	XOJ_TYPE_ATTRIB;

	int i;
	int count;
	const T * data;
};

#endif /* __ARRAYITERATOR_H__ */

