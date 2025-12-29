#pragma once
#include <vector>
#include <atomic>

struct PatternState
{
    std::atomic<int> currentStep{0};
    std::atomic<bool> isPlaying{false};

    // For UI visualization - updated from audio thread via lock-free mechanism
    // In MVP, UI can poll currentStep for display
};
