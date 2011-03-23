/*
 * Xournal++
 *
 * Used for testing memory violations
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __MEMORYCHECK_H__
#define __MEMORYCHECK_H__

#include <assert.h>

class MemoryCheckObject {
public:
	MemoryCheckObject();

	bool isMemoryCorrupted(const char * file, int line);
private:
	int d1;
	int d2;
	int d3;
};

#define CHECK_MEMORY(obj) {	bool corrupted = obj->isMemoryCorrupted(__FILE__, __LINE__); \
	if(corrupted) { \
		fprintf(stderr, "%s:%i\tMemory corrupted!\n", __FILE__, __LINE__); \
	} \
	assert(!corrupted); }


#endif /* __MEMORYCHECK_H__ */
