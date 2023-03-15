#pragma once

#include "parameters.hpp"
#include "dsp/gru.hpp"
#include "dsp/envelope.hpp"
#include "gui/editor_state.hpp"

inline juce::RangedAudioParameter*
getParameterByName(juce::AudioProcessor& processor, juce::String name) {
    for (auto& parameter : processor.getParameters()) {
        if (parameter->getName(255) == name) {
            return dynamic_cast<juce::RangedAudioParameter*>(parameter);
        }
    }
    // parameter not found
    jassertfalse;
    return nullptr;
}

class BenderProcessor : public PluginHelpers::ProcessorBase
{
public:
    BenderProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    EditorState& getEditorState() {
        return editorState;
    }

    BenderParameters& getBenderParameters() {
        return benderParameters;
    }

private:
    
    int64_t tick{ 0 };

    static constexpr int MatrixUpdatePeriod = 1024;
    static constexpr int DecibelsUpdatePeriod = 16;
    static constexpr int HiddenSize = 8;

    std::array<dsp::Gru<4, HiddenSize>, 2> first_filter;
    std::array<dsp::Gru<4, HiddenSize>, 2> second_filter;

    dsp::Gru<4, 8>::Model::HiddenToHidden bendingMatrixFirstFirst{};
    dsp::Gru<4, 8>::Model::HiddenToHidden bendingMatrixFirstSecond{};
    dsp::Gru<4, 8>::Model::HiddenToHidden bendingMatrixSecondFirst{};
    dsp::Gru<4, 8>::Model::HiddenToHidden bendingMatrixSecondSecond{};


    std::array<float, 2> in_rms{ 0 };
    std::array<float, 2> middle_rms{ 0 };
    std::array<float, 2> out_rms{ 0 };

    float rms_smooth_alpha{ 0.99f };
    float param_smooth_alpha{ 0.99f };
    float decibels_smooth_alpha{ 0.99f };

    float smoothed_drive{ 0 };
    float smoothed_mix{ 0 };
    float smoothed_drive2{ 0 };
    float smoothed_mix2{ 0 };
    float smoothed_autogain{ 0 };
    float smoothed_output{ 0 };

    float logSampleRateFactor{ 0 };

    dsp::Envelope envelope;
    std::vector<float> envelopeWorkbuf;

    BenderParameters benderParameters;

    EditorState editorState{ *this, 900, 800 };

    // Turn off envelope module until the first midi command revieved
    bool midiMode{ false };

};
