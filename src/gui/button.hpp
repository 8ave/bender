#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "uiData.h"
#include "svg.hpp"
#include "sub_component.hpp"

class DrawableButtonWithDefault : public juce::DrawableButton {
public:
	DrawableButtonWithDefault()
		: juce::DrawableButton("", juce::DrawableButton::ImageStretched) {
	}
};

class Button : public SubComponent<DrawableButtonWithDefault> {
public:

	Button(juce::Rectangle<float> position_,
	       EditorState& editorState_,
		   juce::Drawable* image_,
		   std::function<void(void)> onClick_)
		   : SubComponent<DrawableButtonWithDefault>(position_, editorState_)
	{
		// DrawableButton makes a copy and owns it
		onClick = std::move(onClick_);
		setImages(image_);
	}

};