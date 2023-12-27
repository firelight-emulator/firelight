//
// Created by nicho on 12/27/2023.
//

#include "fps_multiplier.hpp"


void FpsMultiplier::stop() {
    std::printf("Stop!");
}

void FpsMultiplier::start() {
    std::printf("Start!");
}

void FpsMultiplier::setSliderValue(double value) {
        if (value == m_sliderValue) {
            return;
        }
        std::printf("Changed value from: %f to %f \r\n", m_sliderValue, value);
        m_sliderValue = value;
        emit sliderValueChanged(m_sliderValue);
    }
