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
	}

	~AudioQueue()
	{
	}

public:
	void reset()
	{
		this->popNotified = false;
		this->pushNotified = false;
		this->streamEnd = false;
		this->clear();

		this->sampleRate = -1;
		this->channels = 0;
	}

	bool empty()
	{
		return std::deque<T>::empty();
	}

	unsigned long size()
	{
		return std::deque<T>::size();
	}

	void push(T* samples, unsigned long nSamples)
	{
		for (unsigned long i = 0; i < nSamples; i++)
		{
			this->push_front(samples[i]);
		}

		this->popNotified = false;

		this->pushNotified = true;
		this->pushLockCondition.notify_one();
	}

	void pop(T* returnBuffer, unsigned long& returnBufferLength, unsigned long nSamples)
	{
		if (this->channels == 0)
		{
			returnBufferLength = 0;

			this->popNotified = true;
			this->popLockCondition.notify_one();

			return;
		}

		returnBufferLength = std::min(nSamples, this->size() - this->size() % this->channels);
		for (long i = 0; i < returnBufferLength; i++)
		{
			returnBuffer[i] = this->back();
			this->pop_back();
		}

		this->pushNotified = false;

		this->popNotified = true;
		this->popLockCondition.notify_one();
	}

	void signalEndOfStream()
	{
		this->streamEnd = true;
		this->pushNotified = true;
		this->pushLockCondition.notify_one();
		this->popNotified = true;
		this->popLockCondition.notify_one();
	}

	void waitForProducer(std::unique_lock<std::mutex>& lock)
	{
		while (!this->pushNotified)
		{
			this->pushLockCondition.wait(lock);
		}
	}

	void waitForConsumer(std::unique_lock<std::mutex>& lock)
	{
		while (!this->popNotified)
		{
			this->popLockCondition.wait(lock);
		}
	}

	bool hasStreamEnded()
	{
		return this->streamEnd;
	}

	std::mutex& syncMutex()
	{
		return this->queueLock;
	}

	void setAudioAttributes(double sampleRate, unsigned int channels)
	{
		this->sampleRate = sampleRate;
		this->channels = channels;
	}

	void getAudioAttributes(double &sampleRate, unsigned int &channels)
	{
		sampleRate = this->sampleRate;
		channels = this->channels;
	}

private:
protected:
	std::mutex queueLock;
	bool streamEnd = false;
	std::condition_variable pushLockCondition;
	bool pushNotified = false;
	std::condition_variable popLockCondition;
	bool popNotified = false;

	double sampleRate = -1;
	unsigned int channels = 0;
};
