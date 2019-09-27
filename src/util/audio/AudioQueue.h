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
class AudioQueue
{
public:
	AudioQueue()
	{
		XOJ_INIT_TYPE(AudioQueue);

		unsigned int size = 1024 * 200;
		buf = (std::unique_ptr<T[]>(new T[size]));
		buf_max_size = size;
	}

	~AudioQueue()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		XOJ_RELEASE_TYPE(AudioQueue);
	}

private:
	std::unique_ptr<T[]> buf;
	size_t buf_head = 0;
	size_t buf_tail = 0;
	bool buf_full = 0;
	int buf_max_size = 0;

public:
	void reset()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		this->popNotified = false;
		this->pushNotified = false;
		this->streamEnd = false;

		buf_head = buf_tail = 0;
		buf_full = false;

		for (long i = 0; i < buf_max_size; i++)
		{
			buf[i] = 0;
		}		

		this->sampleRate = -1;
		this->channels = 0;

	}

	bool empty()
	{
		XOJ_CHECK_TYPE(AudioQueue);

	    return (!buf_full && (buf_head == buf_tail));
	}

	unsigned long size() const
	{
		size_t size = buf_max_size;

		if(!buf_full)
		{
			if(buf_head >= buf_tail)
			{
				size = buf_head - buf_tail;
			}
			else
			{
				size = buf_max_size + buf_head - buf_tail;
			}
		}

		return size;
	}

	void push(T* samples, unsigned long nSamples)
	{
		XOJ_CHECK_TYPE(AudioQueue);

		for (unsigned long i = 0; i < nSamples; i++)
		{
			buf[buf_head] = samples[i];

			if(buf_full)
			{
				buf_tail = (buf_tail + 1) % buf_max_size;
			}

			buf_head = (buf_head + 1) % buf_max_size;
		    buf_full = buf_head == buf_tail;
		}
		this->popNotified = false;

		this->pushNotified = true;
		this->pushLockCondition.notify_one();
	}

	void pop(T* returnbuf, unsigned long& returnbufLength, unsigned long nSamples)
	{
		XOJ_CHECK_TYPE(AudioQueue);

		if (this->channels == 0)
		{
			returnbufLength = 0;

			this->popNotified = true;
			this->popLockCondition.notify_one();

			return;
		}

		returnbufLength = std::min(nSamples, this->size() - this->size() % this->channels);
		for (long i = 0; i < returnbufLength; i++)
		{
			returnbuf[i] = buf[buf_tail];
			buf_tail = (buf_tail + 1) % buf_max_size;
		}
		buf_full = false;

		this->pushNotified = false;

		this->popNotified = true;
		this->popLockCondition.notify_one();
	}

	void signalEndOfStream()
	{
		XOJ_CHECK_TYPE(AudioQueue);

		this->streamEnd = true;
		this->pushNotified = true;
		this->pushLockCondition.notify_one();
		this->popNotified = true;
		this->popLockCondition.notify_one();
	}

	void waitForProducer(std::unique_lock<std::mutex>& lock)
	{
		XOJ_CHECK_TYPE(AudioQueue);

		while (!this->pushNotified)
		{
			this->pushLockCondition.wait(lock);
		}
	}

	void waitForConsumer(std::unique_lock<std::mutex>& lock)
	{
		XOJ_CHECK_TYPE(AudioQueue);

		while (!this->popNotified)
		{
			this->popLockCondition.wait(lock);
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
	XOJ_TYPE_ATTRIB;

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
