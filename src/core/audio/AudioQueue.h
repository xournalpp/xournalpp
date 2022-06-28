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

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <deque>
#include <limits>
#include <mutex>

template <typename T>
class AudioQueue {
public:
    void reset() {
        std::lock_guard<std::mutex> lock(internalLock);
        this->popNotified = false;
        this->pushNotified = false;
        this->streamEnd = false;
        internalQueue.clear();

        this->sampleRate = -1;
        this->channels = 0;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(internalLock);
        return internalQueue.empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(internalLock);
        return internalQueue.size();
    }

    template <typename Iter>
    void emplace(Iter begI, Iter endI) {
        std::lock_guard<std::mutex> lock(internalLock);
        std::move(begI, endI, std::front_inserter(internalQueue));

        this->pushNotified = true;
        this->pushLockCondition.notify_one();
    }

    template <typename InsertIter>
    InsertIter pop(InsertIter insertIter, size_t nSamples) {
        std::lock_guard<std::mutex> lock(internalLock);

        if (this->channels == 0) {
            this->popNotified = true;
            this->popLockCondition.notify_one();
            return insertIter;
        }


        auto queueSize = internalQueue.size();
        auto returnBufferLength = std::min<size_t>(nSamples, queueSize - queueSize % this->channels);
        auto begI = rbegin(internalQueue);
        auto endI = std::next(begI, returnBufferLength);

        auto ret = std::move(begI, endI, insertIter);
        internalQueue.erase(endI.base(), begI.base());

        this->popNotified = true;
        this->popLockCondition.notify_one();
        return ret;
    }

    void signalEndOfStream() {
        std::lock_guard<std::mutex> lock(internalLock);
        this->streamEnd = true;
        this->pushNotified = true;
        this->popNotified = true;
        this->pushLockCondition.notify_one();
        this->popLockCondition.notify_one();
    }

    void waitForProducer(std::unique_lock<std::mutex>& lock) {
        // static_assert(lock.mutex() == &this->queueLock);
        assert(lock.mutex() == &this->queueLock);
        while (!this->pushNotified && !hasStreamEnded()) { this->pushLockCondition.wait(lock); }
        this->pushNotified = false;
    }

    void waitForConsumer(std::unique_lock<std::mutex>& lock) {
        // static_assert(lock.mutex() == &this->queueLock);
        assert(lock.mutex() == &this->queueLock);
        while (!this->popNotified && !hasStreamEnded()) { this->popLockCondition.wait(lock); }
        this->popNotified = false;
    }

    bool hasStreamEnded() {
        std::lock_guard<std::mutex> lock(internalLock);
        return this->streamEnd;
    }

    [[nodiscard]] std::unique_lock<std::mutex> acquire_lock() {
        std::unique_lock retLock{this->queueLock, std::defer_lock};
        std::lock(retLock, this->internalLock);
        std::lock_guard{this->internalLock, std::adopt_lock};
        return retLock;
    }

    void setAudioAttributes(double lSampleRate, unsigned int lChannels) {
        std::lock_guard<std::mutex> lock(internalLock);
        this->sampleRate = lSampleRate;
        this->channels = lChannels;
    }

    /**
     * @return a pair of the sample rate and the channel count,
     * std::pair<>::first is the sample rate and std::pair<>::second the channel count.
     * Todo (readability, type-safety): create a struct AudioAttributes; remove this comment
     */

    [[nodiscard]] std::pair<double, uint32_t> getAudioAttributes() {
        std::lock_guard<std::mutex> lock(internalLock);
        return {this->sampleRate, this->channels};
    }

private:
    std::mutex queueLock;
    std::mutex internalLock;

    std::deque<T> internalQueue;

    std::condition_variable pushLockCondition;
    std::condition_variable popLockCondition;

    double sampleRate{std::numeric_limits<double>::quiet_NaN()};
    uint32_t channels{0};

    bool streamEnd{false};
    bool pushNotified{false};
    bool popNotified{false};
};
