#pragma once
#include <vector>
#include <random>

class PitchGenerator
{
public:
    int generatePitch(int rootNote, int scaleIndex, int octaveRange,
                      int step, int seed)
    {
        // Seed the generator deterministically based on step + seed
        std::mt19937 rng(seed + step * 127);

        const auto& scale = scales[scaleIndex];
        int scaleSize = static_cast<int>(scale.size());

        // Pick random scale degree
        std::uniform_int_distribution<int> scaleDist(0, scaleSize - 1);
        int degree = scaleDist(rng);

        // Pick octave
        std::uniform_int_distribution<int> octaveDist(0, octaveRange - 1);
        int octave = octaveDist(rng);

        // Weight toward root note on beat 1
        if (step == 0)
        {
            std::uniform_real_distribution<float> rootChance(0.0f, 1.0f);
            if (rootChance(rng) < 0.7f) // 70% chance of root on beat 1
            {
                degree = 0;
                octave = 0;
            }
        }

        return rootNote + scale[degree] + (octave * 12);
    }

private:
    // Scale intervals from root
    const std::vector<std::vector<int>> scales = {
        {0, 3, 5, 7, 10},           // Minor Pentatonic
        {0, 2, 4, 5, 7, 9, 11},     // Major
        {0, 2, 3, 5, 7, 8, 10},     // Minor (Natural)
        {0, 2, 3, 5, 7, 9, 10},     // Dorian
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11} // Chromatic
    };
};
