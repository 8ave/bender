#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "processor.hpp"
#include "editor_state.hpp"

// SubComponent uses CRTP to inject methods into vanilla Juce hierarchy
// It is used to inject window auto-resize capabilities
// editorState structure contains the information
// about how the component should be scaled

template<typename T>
class SubComponent : public T {

	EditorState& editorState;
	juce::Rectangle<float> position;
	juce::Rectangle<float> scaled_position;
	juce::Rectangle<float> local_scaled_position;

public:

	SubComponent(juce::Rectangle<float> position_, EditorState& editorState_)
		: position(position_)
		, editorState(editorState_) {
		resize();
	}

	juce::Rectangle<float> getScaledPosition() {
		return scaled_position;
	}

	juce::Rectangle<float> getLocalScaledPosition() {
		return local_scaled_position;
	}

	void resize() {
		float coefX = editorState.width / editorState.baseWidth;
		float coefY = editorState.height / editorState.baseHeight;
		float newX = position.getX() * coefX;
		float newY = position.getY() * coefY;
		float newWidth = position.getWidth() * coefX;
		float newHeight = position.getHeight() * coefY;
		scaled_position = juce::Rectangle<float>(newX, newY, newWidth, newHeight);
		local_scaled_position = getLocalArea(getParentComponent(), getScaledPosition());
		setBounds(scaled_position.toNearestInt());
	}
};
