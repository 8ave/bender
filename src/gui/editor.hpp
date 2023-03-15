#pragma once

#include "processor.hpp"
#include "knob.hpp"
#include "button.hpp"

class BenderEditor : public juce::AudioProcessorEditor
{
public:
    explicit BenderEditor(BenderProcessor&);

private:
    void paint(juce::Graphics&) override;
    void resized() override;

    std::unique_ptr<juce::Drawable> background{ svg::load(uiData::layout3x_svg) };
    juce::Component* help{ nullptr };

    EditorState& state;

    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<std::unique_ptr<Button>> buttons;
};
