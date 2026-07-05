#pragma once

/**
 * Controls the audio rate compensation for a libretro core based on the fill level of the audio sink buffer.
*/
class AudioRateController {
public:
    /**
     * Calculates the compensation value based on the current fill level of the audio sink buffer.
     * Can be used with AudioResampler to adjust the audio rate dynamically to prevent buffer underruns or overruns.
     */
    int computeCompensation(int usedBytes, int bufferCapacityBytes);

    /**
     * Resets the internal state of the audio rate controller, clearing any previous usage data.
     */
    void reset();

private:
    static constexpr int WINDOW_SIZE = 10;
    int m_usageBytes[WINDOW_SIZE] = {};
    int m_index = 0;
    int m_populatedCount = 0;
    double m_previousAvgFillRatio = -1.0;
};
