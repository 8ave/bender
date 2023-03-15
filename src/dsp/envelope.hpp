#pragma once

#include <vector>

namespace dsp {

// Simple linear envelope

class Envelope {
public:

    enum class State { Idle, Attack, Decay, Sustain, Release };

    static constexpr float minValue{ 0.2f };
    static constexpr float maxValue{ 1.0f };

    void processReplacing(std::vector<float>& buf,
                          float samplerate,
                          float attack,
                          float decay,
                          float sustain,
                          float release) {
        size_t total_samples = buf.size();

        float dattack = 1.0f / (attack * float(samplerate));
        float ddecay = (1.0f - sustain) / (decay * float(samplerate));
        float drelease = sustain / (release * float(samplerate));

        for(size_t i = 0; i < total_samples; ++i) {
            if (buf[i] == 1.0f) {
                state = State::Attack;
            }

            if (buf[i] == -1.0f) {
                state = State::Release;
            }

            if (state == State::Idle) {
                value = 0.0f;
            } else if (state == State::Attack) {
                if (value < 1) {
                    value += dattack;
                } else {
                    state = State::Decay;
                }
            } else if (state == State::Decay) {
                if (value > sustain) {
                    value = value - ddecay;
                } else {
                    value = sustain;
                    state = State::Sustain;
                }
            } else if (state == State::Sustain) {
                // do nothing
            } else if (state == State::Release) {
                if (value > 0) {
                    value = value - drelease;
                } else {
                    value = 0;
                    state = State::Idle;
                }
            } else {
                assert(false);
            }

            buf[i] = value * (maxValue - minValue) + minValue;

        }
    }
private:

    float value{ 0.0f };
    State state{ State::Idle };

};

}  // namespace dsp
