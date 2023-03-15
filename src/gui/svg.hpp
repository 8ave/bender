#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace svg {

class BuildingError : public std::exception {
	std::string _msg;
public:
	BuildingError(const std::string& msg) : _msg(msg) {}

	const char* what() const noexcept override
	{
		return _msg.c_str();
	}
};

inline std::unique_ptr<juce::Drawable> load(const char* data) {
	std::unique_ptr<juce::Drawable> drawable;

	auto svg = juce::XmlDocument::parse(data);
	if (!svg) {
		throw BuildingError("Can't parse xml from cstring");
	}

	drawable = juce::DrawableComposite::createFromSVG(*svg);
	if (!drawable) {
		throw BuildingError("Can't create DrawableComposite from svg");
	}

	return drawable;
}

}
