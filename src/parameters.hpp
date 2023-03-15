#pragma once

#include "processor_base.hpp"

struct BenderParameters
{
    //Raw pointers. They will be owned by either the processor or the APVTS (if you use it)
    juce::AudioParameterFloat* cutoff =
        new juce::AudioParameterFloat({"Cutoff", 1}, "Cutoff", 0.f, 1.f, 0.8f);

    juce::AudioParameterFloat* res =
        new juce::AudioParameterFloat({"Res", 1}, "Res", 0.f, 1.f, 0.4f);

    juce::AudioParameterFloat* drive =
        new juce::AudioParameterFloat({"Drive", 1}, "Drive", -18.0f, 18.0f, 0.0f);

    juce::AudioParameterFloat* mix =
        new juce::AudioParameterFloat({ "Mix", 1 }, "Mix", 0.f, 1.f, 1.0f);

    juce::AudioParameterFloat* cutoff2 =
        new juce::AudioParameterFloat({"Cutoff2", 1}, "Cutoff2", 0.f, 1.f, 0.5f);

    juce::AudioParameterFloat* res2 =
        new juce::AudioParameterFloat({"Res2", 1}, "Res2", 0.f, 1.f, 0.5f);

    juce::AudioParameterFloat* drive2 =
        new juce::AudioParameterFloat({"Drive2", 1}, "Drive2", -18.0f, 18.0f, 0.0f);

    juce::AudioParameterFloat* mix2 =
        new juce::AudioParameterFloat({ "Mix2", 1 }, "Mix2", 0.f, 1.f, 0.0f);

    juce::AudioParameterFloat* f1selfMain =
        new juce::AudioParameterFloat({ "F1SelfMain", 1 }, "F1SelfMain", -0.8f, 0.8f, 0.4f);
    juce::AudioParameterFloat* f1crossMain =
        new juce::AudioParameterFloat({ "F1CrossMain", 1 }, "F1CrossMain", -0.8f, 0.8f, 0.4f);
    juce::AudioParameterFloat* f2selfMain =
        new juce::AudioParameterFloat({ "F2SelfMain", 1 }, "F2SelfMain", -0.8f, 0.8f, 0.4f);
    juce::AudioParameterFloat* f2crossMain =
        new juce::AudioParameterFloat({ "F2CrossMain", 1 }, "F2CrossMain", -0.8f, 0.8f, 0.4f);

    juce::AudioParameterFloat* bend =
        new juce::AudioParameterFloat({ "Bend", 1 }, "Bend", -0.8f, 0.8f, 0.3f);

    juce::AudioParameterFloat* autoGain =
        new juce::AudioParameterFloat({ "AutoGain", 1 }, "AutoGain", 0.f, 1.f, 0.5f);

    juce::AudioParameterFloat* output =
        new juce::AudioParameterFloat({ "Output", 1 }, "Output", -24.0f, 24.0f, 0.0f);

    juce::AudioParameterFloat* attack =
        new juce::AudioParameterFloat({ "Attack", 1 }, "Attack", 0.01f, 1.f, 0.01f);
    juce::AudioParameterFloat* decay =
        new juce::AudioParameterFloat({ "Decay", 1 }, "Decay", 0.01f, 1.f, 0.2f);
    juce::AudioParameterFloat* sustain =
        new juce::AudioParameterFloat({ "Sustain", 1 }, "Sustain", 0.f, 1.f, 0.99f);
    juce::AudioParameterFloat* release =
        new juce::AudioParameterFloat({ "Release", 1 }, "Release", 0.01f, 1.f, 0.2f);


    std::vector<std::vector<size_t>> self_elems_to_use =
    { { 0, 4 }, { 1, 0 }, { 1, 2 }, { 1, 4 },
      { 2, 0 }, { 2, 1 }, { 2, 4 }, { 3, 4 },
      { 4, 0 }, { 4, 1 }, { 6, 1 }, { 6, 3 },
      { 6, 4 }, { 6, 5 }, { 7, 0 }, { 7, 4 }
    };

    std::vector<std::vector<size_t>> cross_elems_to_use =
    { { 0, 8 - 8 }, { 0, 12 - 8 }, { 1, 12 - 8 }, { 2, 8 - 8 },
      { 2, 12 - 8 }, { 2, 15 - 8 }, { 3, 9 - 8 }, { 3, 12 - 8 },
      { 4, 12 - 8 }, { 6, 8 - 8 }, { 6, 9 - 8 }, { 6, 10 - 8 },
      { 6, 12 - 8 }, { 6, 13 - 8 }, { 7, 11 - 8 }, { 7, 12 - 8 }
    };

    static std::vector<juce::AudioParameterFloat*> makeMatrix(const char* prefix,
                                                              const std::vector<std::vector<size_t>>& elems) {
        std::vector<juce::AudioParameterFloat*> result;

        for (auto& elem : elems) {
            size_t i = elem[0];
            size_t j = elem[1];
            juce::String matrix_str = prefix + juce::String(i) + "_" + juce::String(j);
            result.emplace_back(new juce::AudioParameterFloat({ matrix_str, 1 }, matrix_str, -1.f, 1.f, 0.f));
        }

        return result;
    }

    std::vector<juce::AudioParameterFloat*> f1self{ makeMatrix("F1Self", self_elems_to_use) };
    std::vector<juce::AudioParameterFloat*> f1cross{ makeMatrix("F1Cross", cross_elems_to_use) };
    std::vector<juce::AudioParameterFloat*> f2self{ makeMatrix("F2Self", self_elems_to_use) };
    std::vector<juce::AudioParameterFloat*> f2cross{ makeMatrix("F2Cross", cross_elems_to_use) };

    void add(juce::AudioProcessor& processor) const
    {
        processor.addParameter(cutoff);
        processor.addParameter(res);
        processor.addParameter(drive);
        processor.addParameter(mix);

        processor.addParameter(cutoff2);
        processor.addParameter(res2);
        processor.addParameter(drive2);
        processor.addParameter(mix2);

        processor.addParameter(f1selfMain);
        processor.addParameter(f1crossMain);
        processor.addParameter(f2selfMain);
        processor.addParameter(f2crossMain);
        processor.addParameter(bend);

        processor.addParameter(autoGain);
        processor.addParameter(output);

        processor.addParameter(attack);
        processor.addParameter(decay);
        processor.addParameter(sustain);
        processor.addParameter(release);

        for (auto& matrixElement : f1self) {
            processor.addParameter(matrixElement);
        }
        for (auto& matrixElement : f1cross) {
            processor.addParameter(matrixElement);
        }
        for (auto& matrixElement : f2self) {
            processor.addParameter(matrixElement);
        }
        for (auto& matrixElement : f2cross) {
            processor.addParameter(matrixElement);
        }



    }

};
