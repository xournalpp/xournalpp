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
#include <algorithm>
#include <condition_variable>

template <typename T>
class AudioQueue : protected std::deque<T>
{
public:
	AudioQueue() = default;

	~AudioQueue() = default;

public:
	void reset()
	{
		std::lock_guard<std::mutex> lock(internalLock);
		this->popNotified = false;
		this->pushNotified = false;
		this->streamEnd = false;
		this->clear();

		this->sampleRate = -1;
		this->channels = 0;
	}

	bool empty()
	{
		std::lock_guard<std::mutex> lock(internalLock);
		return std::deque<T>::empty();
	}

	size_t size()
	{
		std::lock_guard<std::mutex> lock(internalLock);
		return std::deque<T>::size();
	}

	void push(T* samples, size_t nSamples)
	{
		{
			std::lock_guard<std::mutex> lock(internalLock);
			for (size_t i = 0; i < nSamples; i++)
			{
				this->push_front(samples[i]);
			}
		}

		this->popNotified = false;

		this->pushNotified = true;
		this->pushLockCondition.notify_one();
	}

	void pop(T* returnBuffer, size_t& returnBufferLength, size_t nSamples)
	{
		if (this->channels == 0)
		{
			returnBufferLength = 0;

			this->popNotified = true;
			this->popLockCondition.notify_one();

			return;
		}

		{
			std::lock_guard<std::mutex> lock(internalLock);
			returnBufferLength = std::min<size_t>(nSamples, std::deque<T>::size() - std::deque<T>::size() % this->channels);
			for (size_t i = 0; i < returnBufferLength; i++)
			{
				returnBuffer[i] = this->back();
				this->pop_back();
			}
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
		std::lock_guard<std::mutex> lock(internalLock);
		this->sampleRate = sampleRate;
		this->channels = channels;
	}

	void getAudioAttributes(double &sampleRate, unsigned int &channels)
	{
		std::lock_guard<std::mutex> lock(internalLock);
		sampleRate = this->sampleRate;
		channels = this->channels;
	}

private:
protected:
	std::mutex queueLock;
	std::mutex internalLock;
	bool streamEnd = false;
	std::condition_variable pushLockCondition;
	bool pushNotified = false;
	std::condition_variable popLockCondition;
	bool popNotified = false;

	double sampleRate = -1;
	unsigned int channels = 0;
};
