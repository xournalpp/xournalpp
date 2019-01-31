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

    void reset();
    bool empty();
    unsigned long size();
    void push(int *samples, unsigned long nSamples);
    std::vector<int> pop(unsigned long nSamples);

    void signalEndOfStream();
    void waitForNewElements();
    bool hasStreamEnded();

private:
    std::mutex queueLock;
    std::unique_lock<std::mutex> lock;
    std::condition_variable lockCondition;
    bool streamEnd = false;
    bool notified = false;
private:
    XOJ_TYPE_ATTRIB;
};


