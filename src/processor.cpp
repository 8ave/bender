#include "processor.hpp"
#include "gui/editor.hpp"

BenderProcessor::BenderProcessor() {
    benderParameters.add(*this);

    bendingMatrixFirstFirst.setZero();
    bendingMatrixFirstSecond.setZero();
    bendingMatrixSecondFirst.setZero();
    bendingMatrixSecondSecond.setZero();

    for (int i = 0; i < HiddenSize; ++i) {
        bendingMatrixFirstFirst(i, i) = 1;
        bendingMatrixSecondSecond(i, i) = 1;
    }

    prepareToPlay(44100.0, 100);
}

void BenderProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(midiMessages);

    jassert(buffer.getNumChannels() == 2);

    std::fill(envelopeWorkbuf.begin(), envelopeWorkbuf.end(), 0.0f);
    for (const juce::MidiMessageMetadata metadata : midiMessages) {
        auto message = metadata.getMessage();
        if (message.isNoteOn()) {
            midiMode = true;
            envelopeWorkbuf[size_t(message.getTimeStamp())] = 1;
        }
        if (message.isNoteOff()) {
            midiMode = true;
            envelopeWorkbuf[size_t(message.getTimeStamp())] = -1;
        }
    }

    envelope.processReplacing(envelopeWorkbuf, float(getSampleRate()),
                              *benderParameters.attack,
                              *benderParameters.decay,
                              *benderParameters.sustain,
                              *benderParameters.release);

    std::array<float*, 2> channelSamples = { buffer.getWritePointer(0),
                                             buffer.getWritePointer(1) };

    for (auto sampleIdx = 0; sampleIdx < buffer.getNumSamples(); ++sampleIdx) {

        if (tick % MatrixUpdatePeriod == 0) {
            size_t param_no = 0;

            float strength = *benderParameters.bend * *benderParameters.f1selfMain;
            for (auto& elem : benderParameters.self_elems_to_use) {
                size_t i = elem[0];
                size_t j = elem[1];
                bendingMatrixFirstFirst(i, j) = *benderParameters.f1self[param_no] * strength;
                ++param_no;
            }

            param_no = 0;
            strength = *benderParameters.bend * *benderParameters.f1crossMain;
            for (auto& elem : benderParameters.cross_elems_to_use) {
                size_t i = elem[0];
                size_t j = elem[1];
                bendingMatrixFirstSecond(i, j) = *benderParameters.f1cross[param_no] * strength;
                ++param_no;
            }

            param_no = 0;
            strength = *benderParameters.bend * *benderParameters.f2selfMain;
            for (auto& elem : benderParameters.self_elems_to_use) {
                size_t i = elem[0];
                size_t j = elem[1];
                bendingMatrixSecondSecond(i, j) = *benderParameters.f2self[param_no] * strength;
                ++param_no;
            }

            param_no = 0;
            strength = *benderParameters.bend * *benderParameters.f2crossMain;
            for (auto& elem : benderParameters.cross_elems_to_use) {
                size_t i = elem[0];
                size_t j = elem[1];
                bendingMatrixSecondFirst(i, j) = *benderParameters.f2cross[param_no] * strength;
                ++param_no;
            }

        }

        float cutoff_compensated = *benderParameters.cutoff - logSampleRateFactor;
        float cutoff2_compensated = *benderParameters.cutoff2 - logSampleRateFactor;
        float cutoffWithEnvelope = midiMode? cutoff_compensated * envelopeWorkbuf[sampleIdx] : cutoff_compensated;
        float cutoff2WithEnvelope = midiMode? cutoff2_compensated * envelopeWorkbuf[sampleIdx] : cutoff2_compensated;

        if (tick % DecibelsUpdatePeriod == 0) {
            smoothed_drive = dsp::lerp(juce::Decibels::decibelsToGain(float(*benderParameters.drive)),
                                       smoothed_drive,
                                       decibels_smooth_alpha);
            smoothed_drive2 = dsp::lerp(juce::Decibels::decibelsToGain(float(*benderParameters.drive2)),
                                        smoothed_drive2,
                                        decibels_smooth_alpha);
            smoothed_output = dsp::lerp(juce::Decibels::decibelsToGain(float(*benderParameters.output)),
                                        smoothed_output,
                                        decibels_smooth_alpha);
        }

        smoothed_autogain = dsp::lerp(*benderParameters.autoGain,
                                      smoothed_autogain,
                                      param_smooth_alpha);
        smoothed_mix = dsp::lerp(*benderParameters.mix,
                                 smoothed_mix,
                                 param_smooth_alpha);
        smoothed_mix2 = dsp::lerp(*benderParameters.mix2,
                                  smoothed_mix2,
                                  param_smooth_alpha);

        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel) {

            float sample = channelSamples[channel][sampleIdx];
            // making signal less hot
            sample = sample * 0.25f;

            in_rms[channel] = dsp::lerp(fabsf(sample),
                                        in_rms[channel],
                                        rms_smooth_alpha);

            float flt1 = first_filter[channel].process<dsp::ApproximateMath2>(sample * smoothed_drive,
                                                                              cutoffWithEnvelope,
                                                                              *benderParameters.res,
                                                                              bendingMatrixFirstFirst,
                                                                              bendingMatrixFirstSecond,
                                                                              second_filter[channel].state.hidden);
            sample = dsp::lerp(sample, flt1, smoothed_mix);

            float flt2 = second_filter[channel].process<dsp::ApproximateMath2>(sample * smoothed_drive2,
                                                                               cutoff2WithEnvelope,
                                                                               *benderParameters.res2,
                                                                               bendingMatrixSecondSecond,
                                                                               bendingMatrixSecondFirst,
                                                                               first_filter[channel].state.hidden);
            sample = dsp::lerp(sample, flt2, smoothed_mix2);

            out_rms[channel] = dsp::lerp(fabsf(sample),
                                         out_rms[channel],
                                         rms_smooth_alpha);
            out_rms[channel] = std::max(out_rms[channel], 1e-8f);

            sample = dsp::lerp(sample,
                               sample * in_rms[channel] / out_rms[channel],
                               smoothed_autogain);

            sample = sample * 4.0f;

            // ear saver
            sample = std::clamp(sample, -2.0f, 2.0f);

            channelSamples[channel][sampleIdx] = sample * smoothed_output;
        }

        ++tick;

    }

}

void BenderProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    rms_smooth_alpha = expf(-2.0f * float(M_PI) * 3.0f / float(sampleRate));
    param_smooth_alpha = expf(-2.0f * float(M_PI) * 4.0f / float(sampleRate));
    decibels_smooth_alpha = expf(-2.0f * float(M_PI) * 4.0f / float(sampleRate) * DecibelsUpdatePeriod);
    envelopeWorkbuf.resize(samplesPerBlock);

    logSampleRateFactor = float( log(sampleRate / 88200.0) * 0.142763554245889 );

    first_filter[0].reset(float(sampleRate));
    first_filter[1].reset(float(sampleRate));
    second_filter[0].reset(float(sampleRate));
    second_filter[1].reset(float(sampleRate));

}

juce::AudioProcessorEditor* BenderProcessor::createEditor() {
    return new BenderEditor(*this);
}

void BenderProcessor::getStateInformation(juce::MemoryBlock& destData) {
    //Serializes your parameters, and any other potential data into an XML:

    juce::ValueTree params("Params");

    for (auto& param: getParameters()) {
        juce::ValueTree paramTree(PluginHelpers::getParamID(param));
        paramTree.setProperty("Value", param->getValue(), nullptr);
        params.appendChild(paramTree, nullptr);
    }

    juce::ValueTree pluginPreset("NeuralBender");
    pluginPreset.appendChild(params, nullptr);

    copyXmlToBinary(*pluginPreset.createXml(), destData);
}

void BenderProcessor::setStateInformation(const void* data,
                                          int sizeInBytes) {
    //Loads your parameters, and any other potential data from an XML:

    auto xml = getXmlFromBinary(data, sizeInBytes);

    if (xml != nullptr) {
        auto preset = juce::ValueTree::fromXml(*xml);
        auto params = preset.getChildWithName("Params");

        for (auto& param: getParameters()) {
            auto paramTree = params.getChildWithName(PluginHelpers::getParamID(param));

            if (paramTree.isValid()) {
                param->setValueNotifyingHost(paramTree["Value"]);
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BenderProcessor();
}
