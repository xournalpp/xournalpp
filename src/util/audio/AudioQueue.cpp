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

    unsigned long requiredSize = this->size() + nSamples;
    if (this->max_size() < requiredSize)
        this->resize(requiredSize);

    for (long i = nSamples - 1; i >= 0; i--)
        this->push_front(samples[i]);
}

std::vector<int> AudioQueue::pop(unsigned long nSamples)
{
    XOJ_CHECK_TYPE(AudioQueue);

    std::vector<int> buffer(nSamples);
    for (long i = nSamples - 1; i >= 0; i--)
    {
        buffer[i] = this->back();
        this->pop_back();
    }
    return buffer;
}
