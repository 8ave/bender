# Neural Bender

An audio effect plugin for Neural Audio Plug-in Competition.

First, it exposes 2 ladder lowpass filters, modelled using neural networks. Both have "Mix" knobs that allow using only one of them, together, or something in between.

Second, it exposes machinery to manually interfere into signals that flow inside both filters. In this case, one or both filters become experimental filtering/distortion/resonant machines.

The machinery consists of 4 sets of 16 knobs to rewire connections inside the filters (filter 1 source -> filter 1 destination, filter 1 source -> filter 2 destination, and 2 more), and 5 macro-knobs that control the strength of signal leakage, one for each of 4 knob sets, and one global. It is hard to predict what the knobs will do, but that's the part of the fun, similar to circuit bending.

There is also a filter envelope that can be triggered by midi note on/off events.

# Build

The project supports cmake presets and downloads 3rd party dependencies using cmake FetchContent module. Run this from the root folder:

Windows:

```
cmake . --preset=ci-windows
cmake --build --preset=ci-windows-build
```

Mac:

```
cmake . --preset=ci-macos
cmake --build --preset=ci-macos-build
```

Linux:

```
cmake . --preset=ci-ubuntu
cmake --build --preset=ci-ubuntu-build
```

It will take some time to load the dependencies (eigen and JUCE) and build juceaide.

VST3 plugin will appear at `build/bender_artefacts/Release/VST3/`.
