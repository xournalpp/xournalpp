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

void AudioQueue::push(int *samples, unsigned long nSamples)
{
    XOJ_CHECK_TYPE(AudioQueue);

    for (long i = nSamples - 1; i >= 0; i--)
    {
        this->push_front(samples[i]);
    }

    this->notified = true;
    this->lockCondition.notify_one();
}

std::vector<int> AudioQueue::pop(unsigned long nSamples)
{
    XOJ_CHECK_TYPE(AudioQueue);

    nSamples = std::min(nSamples, this->size());
    std::vector<int> buffer(nSamples);
    for (long i = nSamples - 1; i >= 0; i--)
    {
        buffer[i] = this->back();
        this->pop_back();
    }
    this->notified = false;
    return buffer;
}

void AudioQueue::signalEndOfStream()
{
    this->streamEnd = true;
    this->notified = true;
    this->lockCondition.notify_one();
}

void AudioQueue::waitForNewElements(std::unique_lock<std::mutex> &lock)
{
    while (!this->notified)
    {
        this->lockCondition.wait(lock);
    }
}

bool AudioQueue::hasStreamEnded()
{
    return this->streamEnd;
}

std::mutex &AudioQueue::syncMutex()
{
    return this->queueLock;
}
