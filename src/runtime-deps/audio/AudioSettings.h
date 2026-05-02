/*
 * Xournal++
 *
 * Class to save audio data in an opus file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <portaudiocpp/PortAudioCpp.hxx>  // for PaDeviceIndex

#include "filesystem.h"  // for path

struct AudioSettings {
    static constexpr PaDeviceIndex AUDIO_INPUT_SYSTEM_DEFAULT = -1;
    static constexpr PaDeviceIndex AUDIO_OUTPUT_SYSTEM_DEFAULT = -1;
    /**
     * The index of the audio device used for recording
     */
    PaDeviceIndex audioInputDevice{};

    /**
     * The index of the audio device used for playback
     */
    PaDeviceIndex audioOutputDevice{};

    /**
     * The sample rate used for recording
     */
    double audioSampleRate{};

    /**
     * The gain by which to amplify the recorded audio samples
     */
    double audioGain{};

    /**
     * The default time by which the playback will seek backwards and forwards
     */
    unsigned int defaultSeekTime{};
};
