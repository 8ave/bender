#include "processor.hpp"
#include "editor.hpp"
#include "uiData.h"

BenderEditor::BenderEditor(
    BenderProcessor& p)
    : AudioProcessorEditor(&p)
    , state(p.getEditorState())
{
    state.baseWidth = float(background->getWidth());
    state.baseHeight = float(background->getHeight());

    setSize(int(state.width), int(state.height));
    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(double(state.width) / double(state.height));

    // load components from SVG layout file
    for (auto& layer : background->getChildren()) {
        if (layer->getName() == "Help") {
            help = layer;
            addChildComponent(help);
            help->setVisible(false);
            continue;
        }

        if (layer->getName() == "knobs") {
            for (auto& component_ : layer->getChildren()) {
                auto component = dynamic_cast<juce::Drawable*>(component_);
                juce::String name = component->getName();
                if (!getParameterByName(processor, name)) {
                    // layout SVG contains a knob with unknown parameter
                    jassertfalse;
                    continue;
                }
                knobs.emplace_back(new Knob(component->getDrawableBounds(), state, name));
                addAndMakeVisible(*knobs.back());
                knobs.back()->resize();
            }
            layer->setVisible(false);
        }

        if (layer->getName() == "buttons") {
            for (auto& component_ : layer->getChildren()) {
                auto component = dynamic_cast<juce::Drawable*>(component_);
                juce::String name = component->getName();
                std::vector<std::pair<juce::String, std::vector<juce::AudioParameterFloat*>*>> parameterLists =
                { {"f1self", &p.getBenderParameters().f1self},
                  {"f1cross", &p.getBenderParameters().f1cross},
                  {"f2self", &p.getBenderParameters().f2self},
                  {"f2cross", &p.getBenderParameters().f2cross} };

                bool found = false;
                for (size_t buttonIdx = 0; buttonIdx < parameterLists.size(); ++buttonIdx) {
                    juce::String& buttonName = parameterLists[buttonIdx].first;
                    std::vector<juce::AudioParameterFloat*>* parameterList = parameterLists[buttonIdx].second;
                    if (name == buttonName + "_init") {
                        buttons.emplace_back(new Button(component->getDrawableBounds(),
                                                        state, component,
                                                        [this, parameterList]() {
                                                            for (auto& parameter : *parameterList) {
                                                                parameter->beginChangeGesture();
                                                                parameter->setValueNotifyingHost(0.5f);
                                                                parameter->endChangeGesture();
                                                                repaint();
                                                            }
                                                        }));
                        found = true;
                    } else if (name == buttonName + "_rnd") {
                        buttons.emplace_back(new Button(component->getDrawableBounds(),
                                                        state, component,
                                                        [this, parameterList]() {
                                                            for (auto& parameter : *parameterList) {
                                                                parameter->beginChangeGesture();
                                                                parameter->setValueNotifyingHost(juce::Random::getSystemRandom().nextFloat());
                                                                parameter->endChangeGesture();
                                                                repaint();
                                                            }
                                                        }));
                        found = true;
                    } else if (name == "help") {
                        buttons.emplace_back(new Button(component->getDrawableBounds(),
                                                        state, component,
                                                        [this]() {
                                                            help->setVisible(!help->isVisible());
                                                        }));
                        found = true;
                    }
                }

                if (!found) {
                    // couldn't find a callback for a button id in SVG layout
                    jassertfalse;
                    continue;
                }

                addAndMakeVisible(*buttons.back());
                buttons.back()->resize();
            }
            layer->setVisible(false);
        }
    }

    resized();

}

void BenderEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    if (background) {
        background->drawWithin(g, bounds, juce::RectanglePlacement::stretchToFit, 1.0f);
    }
}

void BenderEditor::resized()
{
    state.width = float(getWidth());
    state.height = float(getHeight());

    for (auto& knob : knobs) {
        knob->resize();
    }

    for (auto& button : buttons) {
        button->resize();
    }

    if (help) {
        help->setBounds(getBounds());
        dynamic_cast<juce::DrawableComposite*>(help)->setBoundingBox(getBounds().toFloat());
    }
}
