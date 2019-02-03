/*
 * Xournal++
 *
 * Class to read audio data from an mp3 file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "AudioQueue.h"
#include "DeviceInfo.h"

#include <sox.h>

#include <thread>
#include <utility>

class SoxProducer
{
public:
    explicit SoxProducer(AudioQueue *audioQueue);
    ~SoxProducer();
    void start(std::string filename, const DeviceInfo &outputDevice, sox_uint64_t timestamp);
    sox_signalinfo_t* getSignalInformation();
    void abort();
    void stop();

protected:protected:
    sox_format_t *inputFile = nullptr;
    bool stopProducer = false;

    AudioQueue *audioQueue;
    std::thread *producerThread;

private:
    XOJ_TYPE_ATTRIB;
};


