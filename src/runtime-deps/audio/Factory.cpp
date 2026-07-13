#include "Factory.h"

xoj::audio::System::System(const AudioSettings& settings, std::function<void()> onStopPlaying):
        recorder(settings), player(settings, std::move(onStopPlaying)) {}

xoj::audio::System::~System() = default;

AudioFactory factory{+[](const AudioSettings& settings, std::function<void()> onStopPlaying) {
    return std::make_unique<xoj::audio::System>(settings, std::move(onStopPlaying));
}};
