/*
 * Xournal++
 *
 * Class to save audio data in an mp3 file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include <control/settings/Settings.h>

#include "AudioQueue.h"
#include "DeviceInfo.h"

#include <sox.h>

#include <thread>
#include <utility>

class SoxConsumer
{
public:
    explicit SoxConsumer(Settings *settings, AudioQueue *audioQueue);
    ~SoxConsumer();

public:
    void start(string filename, unsigned int inputChannels);
    void join();
    void stop();

private:
    XOJ_TYPE_ATTRIB;

protected:
    sox_signalinfo_t* inputSignal = nullptr;
    sox_format_t* outputFile = nullptr;
    bool stopConsumer = false;

    Settings* settings = nullptr;
    AudioQueue* audioQueue = nullptr;
    std::thread* consumerThread = nullptr;
};


