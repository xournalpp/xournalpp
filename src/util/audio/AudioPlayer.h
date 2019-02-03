/*
 * Xournal++
 *
 * Class to play audio from a MP3-file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include "AudioQueue.h"
#include "PortAudioConsumer.h"
#include "SoxProducer.h"

#include <control/settings/Settings.h>

class AudioPlayer
{
public:
    explicit AudioPlayer(Settings *settings);
    ~AudioPlayer();
    void start(std::string filename, unsigned int timestamp = 0);
    void stop();
    void abort();

    vector<DeviceInfo> getOutputDevices();

protected:
    Settings* settings;

    AudioQueue *audioQueue;
    PortAudioConsumer *portAudioConsumer;
    SoxProducer *soxProducer;
    std::thread stopThread;

    XOJ_TYPE_ATTRIB;
};


