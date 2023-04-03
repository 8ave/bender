# Neural Bender

This is an audio effect plugin designed for the Neural Audio Plug-in Competition.

First, Neural Bender features 2 ladder lowpass filters, both modeled using neural networks. Each filter has a "Mix" knob that allows you to use only one filter, both filters together, or a mix of the two.

Second, Neural Bender provides machinery that allows you to manually interfere with the signals flowing inside both filters. When you use this feature, one or both filters become experimental filtering/distortion/resonant machines. The machinery includes 4 sets of 16 knobs to rewire connections inside the filters (filter 1 source -> filter 1 destination, filter 1 source -> filter 2 destination, and 2 more), and 5 macro-knobs that control the strength of signal leakage, with one for each of the 4 knob sets and one global control. It's hard to predict what the knobs will do, but that's the part of the fun, and it's similar to circuit bending.

Finally, Neural Bender includes a filter envelope that can be triggered by midi note on/off events.

# Binaries

See https://github.com/8ave/bender/releases/ . At the moment only win64 build is ready.

# Building

To build Neural Bender, you will need a system with compiler/cmake/JUCE dependencies installed. If you're not sure how to do this, see the Windows/macOS/Linux sections at https://tobanteaudio.gitbook.io/juce-cookbook/setup.

After building, the VST3 plugin will appear at build/bender_artefacts/Release/VST3/.

The project supports cmake presets and downloads 3rd party dependencies using the cmake FetchContent module. It may take some time to load the dependencies (eigen and JUCE) and build juceaide, depending on your Internet connection. Using local JUCE builds is possible; see the `dev-common` preset in `CMakeUserPresets.json`.

## Windows

To build Neural Bender on Windows, run the following commands in a terminal:

```
cmake . --preset=ci-windows -Wno-error=dev
cmake --build --preset=ci-windows-build
```

## MacOS build

To build Neural Bender on MacOS, run the following commands in a terminal:

```
cmake . --preset=ci-macos -Wno-error=dev -G "Unix Makefiles"
cmake --build --preset=ci-macos-build
```

If you have ninja installed, you can omit the -G parameter:

```
cmake . --preset=ci-macos -Wno-error=dev
cmake --build --preset=ci-macos-build
```

## Linux build

To build Neural Bender on Linux, run the following commands in a terminal:

```
cmake . --preset=ci-ubuntu -Wno-error=dev # or add -G "Unix Makefiles" if you don't have ninja
cmake --build --preset=ci-ubuntu-build
```

# Acknowledgments

* Audio Programmer Discord for inspiration
* Eyal Amir for JUCE CMake Repo Prototype
