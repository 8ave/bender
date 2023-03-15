#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "uiData.h"
#include "svg.hpp"
#include "sub_component.hpp"

class Knob : public SubComponent<juce::Slider> {
public:

	juce::SliderParameterAttachment attachment;

	std::unique_ptr<juce::Drawable> background;
	std::unique_ptr<juce::Drawable> front;

	Knob(juce::Rectangle<float> position_, EditorState& editorState_, juce::String parameterName)
		: SubComponent<juce::Slider>(position_, editorState_)
		, attachment(*getParameterByName(editorState_.processor, parameterName), *this, nullptr)
		, background(svg::load(uiData::PuffyKnob_background_svg))
		, front(svg::load(uiData::PuffyKnob_knob_svg))
	{
		setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
		setDoubleClickReturnValue(true,
								  getParameterByName(editorState_.processor, parameterName)->getDefaultValue(),
								  juce::ModifierKeys::noModifiers);
	}

	void paint(juce::Graphics& g) override {
		auto bounds = getLocalBounds().toFloat();
		background->drawWithin(g, bounds, juce::RectanglePlacement::stretchToFit, 1.0f);

		auto min = getMinimum();
		auto max = getMaximum();
		auto pos = (getValue() - min) / (max - min);

		float angle = dsp::lerp(float(-M_PI + M_PI/4.0), float(M_PI - M_PI/4.0), float(pos));

		// juce bug? When angle is exactly 0, the picture is not updated
		if (angle == 0.0f) {
			angle = 1e-8f;
		}

		front->setDrawableTransform(juce::AffineTransform::rotation(angle,
																	float(front->getWidth())/2.0f,
																	float(front->getHeight())/2.0f));
		front->drawWithin(g, bounds,
						  juce::RectanglePlacement::stretchToFit, 1.0f);
	}

};
