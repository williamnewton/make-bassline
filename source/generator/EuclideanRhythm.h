#pragma once
#include <vector>

class EuclideanRhythm
{
public:
    // Bjorklund's algorithm - returns true if step should trigger
    bool shouldTrigger(int step, int steps, int hits, int rotation) const
    {
        if (hits <= 0 || steps <= 0)
            return false;
        if (hits >= steps)
            return true;

        // Apply rotation
        int rotatedStep = (step - rotation + steps) % steps;

        // Bjorklund's algorithm inline
        // Check if this step has a hit using the mathematical pattern
        int bucket = 0;
        for (int i = 0; i < rotatedStep; ++i)
        {
            bucket += hits;
            if (bucket >= steps)
                bucket -= steps;
        }

        int prevBucket = bucket;
        bucket += hits;
        bool trigger = (bucket >= steps) != (prevBucket >= steps) || rotatedStep == 0;

        // More accurate Euclidean check
        return ((rotatedStep * hits) % steps) < hits;
    }

    // Generate full pattern for UI visualization
    std::vector<bool> generatePattern(int steps, int hits, int rotation) const
    {
        std::vector<bool> pattern(steps, false);
        for (int i = 0; i < steps; ++i)
        {
            pattern[i] = shouldTrigger(i, steps, hits, rotation);
        }
        return pattern;
    }
};
