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

template <typename T>
class AudioQueue : protected std::deque<T>
{
public:
	AudioQueue()
	{
		XOJ_INIT_TYPE(AudioQueue);
	}

	~AudioQueue()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		XOJ_RELEASE_TYPE(AudioQueue);
	}

public:
	void reset()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		this->notified = false;
		this->streamEnd = false;
		this->clear();
	}

	bool empty()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		return std::deque<T>::empty();
	}

	unsigned long size()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		return std::deque<T>::size();
	}

	void push(T* samples, unsigned long nSamples)
	{
		XOJ_CHECK_TYPE(AudioQueue);

		for (long i = 0; i < nSamples; i++)
		{
			this->push_front(samples[i]);
		}

		this->notified = true;
		this->lockCondition.notify_one();
	}

	void pop(T* returnBuffer, int* bufferLength, unsigned long nSamples, int numChannels)
	{
		XOJ_CHECK_TYPE(AudioQueue);

		*bufferLength = std::min(nSamples, this->size() - this->size() % numChannels);
		for (long i = 0; i < *bufferLength; i++)
		{
			returnBuffer[i] = this->back();
			this->pop_back();
		}
		this->notified = false;
	}

	void signalEndOfStream()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		this->streamEnd = true;
		this->notified = true;
		this->lockCondition.notify_one();
	}

	void waitForNewElements(std::unique_lock<std::mutex>& lock)
	{
		XOJ_CHECK_TYPE(AudioQueue);

		while (!this->notified)
		{
			this->lockCondition.wait(lock);
		}
	}

	bool hasStreamEnded()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		return this->streamEnd;
	}

	std::mutex& syncMutex()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		return this->queueLock;
	}

private:
	XOJ_TYPE_ATTRIB;

protected:
	std::mutex queueLock;
	std::condition_variable lockCondition;
	bool streamEnd = false;
	bool notified = false;
};
