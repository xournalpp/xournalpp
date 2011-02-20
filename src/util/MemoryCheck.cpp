#include "MemoryCheck.h"
#include "CrashHandler.h"
#include <stdlib.h>

MemoryCheckObject::MemoryCheckObject() {
	this->d1 = 0xffff0000;
	this->d2 = 465456;
	this->d3 = 89535395;
}

bool MemoryCheckObject::isMemoryCorrupted(const char * file, int line) {
	crashHandlerMemoryCorruptionFile = file;
	crashHandlerMemoryCorruptionLine = line;
	bool wrong = (this->d1 != 0xffff0000 || this->d2 != 465456 || this->d3 != 89535395);

	if (!wrong) {
		crashHandlerMemoryCorruptionFile = NULL;
		crashHandlerMemoryCorruptionLine = -1;
	}

	return wrong;
}
