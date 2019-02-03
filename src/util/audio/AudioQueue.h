/*
 * Xournal++
 *
 * Queue to connect an audio producer and an audio consumer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>

class AudioQueue : protected std::deque<int>
{
public:
	AudioQueue();
	~AudioQueue();

public:
	void reset();
	bool empty();
	unsigned long size();
	void push(int* samples, unsigned long nSamples);
	void pop(int* returnBuffer, int* bufferLength, unsigned long nSamples, int numChannels);

	void signalEndOfStream();
	void waitForNewElements(std::unique_lock<std::mutex>& lock);
	bool hasStreamEnded();
	std::mutex& syncMutex();

private:
	XOJ_TYPE_ATTRIB;

protected:
	std::mutex queueLock;
	std::condition_variable lockCondition;
	bool streamEnd = false;
	bool notified = false;
};


