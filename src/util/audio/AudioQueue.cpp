#include "AudioQueue.h"

AudioQueue::AudioQueue()
{
	XOJ_INIT_TYPE(AudioQueue);
}

AudioQueue::~AudioQueue()
{
	XOJ_CHECK_TYPE(AudioQueue);

	XOJ_RELEASE_TYPE(AudioQueue);
}

void AudioQueue::reset()
{
	XOJ_CHECK_TYPE(AudioQueue);

	this->notified = false;
	this->streamEnd = false;
	this->clear();
}

bool AudioQueue::empty()
{
	XOJ_CHECK_TYPE(AudioQueue);

	return deque<int>::empty();
}

unsigned long AudioQueue::size()
{
	XOJ_CHECK_TYPE(AudioQueue);

	return deque<int>::size();
}

void AudioQueue::push(int* samples, unsigned long nSamples)
{
	XOJ_CHECK_TYPE(AudioQueue);

	for (long i = 0; i < nSamples; i++)
	{
		this->push_front(samples[i]);
	}

	this->notified = true;
	this->lockCondition.notify_one();
}

void AudioQueue::pop(int* returnBuffer, int* bufferLength, unsigned long nSamples, int numChannels)
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

void AudioQueue::signalEndOfStream()
{
	XOJ_CHECK_TYPE(AudioQueue);

	this->streamEnd = true;
	this->notified = true;
	this->lockCondition.notify_one();
}

void AudioQueue::waitForNewElements(std::unique_lock<std::mutex> &lock)
{
	XOJ_CHECK_TYPE(AudioQueue);

	while (!this->notified)
	{
		this->lockCondition.wait(lock);
	}
}

bool AudioQueue::hasStreamEnded()
{
	XOJ_CHECK_TYPE(AudioQueue);

	return this->streamEnd;
}

std::mutex &AudioQueue::syncMutex()
{
	XOJ_CHECK_TYPE(AudioQueue);

	return this->queueLock;
}
