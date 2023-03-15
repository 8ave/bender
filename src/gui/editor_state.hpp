#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

struct EditorState {
	juce::AudioProcessor& processor;
	float baseWidth;
	float baseHeight;
	float width;
	float height;

	EditorState(juce::AudioProcessor& processor_, float initWidth, float initHeight)
		: processor(processor_)
		, baseWidth(initWidth)
	    , baseHeight(initHeight)
	    , width(initWidth)
	    , height(initHeight) {
	}
};
